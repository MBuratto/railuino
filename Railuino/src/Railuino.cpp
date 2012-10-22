/*********************************************************************
 * Railuino - Hacking your Marklin
 *
 * (c) 2012 Joerg Pleumann
 * 
 * TODO: Add proper license text here.
 */

//#include <can.h>
#include "Railuino.h"
//#include "canbus/canbus.c"
#include "can/mcp2515.h"
#include "can/mcp2515.c"
#include "ir/infrared.c"

void printHex(byte b) {
	Serial.print("0x");
	if (b < 0x10) {
		Serial.print("0");
	}
	Serial.print(b, HEX);
	Serial.print(" ");
}

#define SIZE 32

#define ulong unsigned long

can_t buffer[SIZE];

volatile int posRead = 0;

volatile int posWrite = 0;

volatile bool lastOpWasWrite = false;

void enqueue() {
	if (posWrite == posRead && lastOpWasWrite) {
		Serial.println("!!! Buffer full");
		return;
	}

	if (can_get_message(&buffer[posWrite])) {
		posWrite = (posWrite + 1) % SIZE;
	} else {
		Serial.println("!!! No message");
	}

	lastOpWasWrite = true;
}

bool dequeue(can_t *p) {
	if (posWrite == posRead && !lastOpWasWrite) {
		return false;
	}

	noInterrupts();

	memcpy(p, &buffer[posRead], sizeof(can_t));
	posRead = (posRead + 1) % SIZE;
	lastOpWasWrite = false;
	interrupts();

	return true;
}

// ===================================================================
// === TrackController ===============================================
// ===================================================================

TrackController::TrackController(int hash, bool debug, bool loopback) {
	mHash = hash;
	mDebug = debug;
	mLoopback = loopback;
}

TrackController::TrackController(int hash, bool debug) {
	TrackController(hash, debug, false);
}

void TrackController::begin() {
	if (!can_init(5, mLoopback)) {
		Serial.println("!!! Init error");
		Serial.println("!!! Emergency stop");
		for (;;);
	}

	attachInterrupt(0, enqueue, FALLING);

	TrackMessage message;

	clearMessage(&message);

	message.command = 0x1b;
	message.length = 0x05;
	message.data[4] = 0x11;

	sendMessage(&message);
}

// end - no interrupts

boolean TrackController::sendMessage(TrackMessage *message) {
	can_t can;

	message->hash = mHash;
	
	can.id = ((uint32_t)message->command) << 17 | (uint32_t)message->hash;
	can.flags.extended = 1;
	can.length = message->length;

	for (int i = 0; i < message->length; i++) {
		can.data[i] = message->data[i];
	}

	if (mDebug) {
	   Serial.print("==> ");
	    printMessage(message);
	}
	
	bool result = can_send_message(&can);

	/*
	Serial.println(can_read_register(0x1c), HEX);
	Serial.println(can_read_register(0x1d), HEX);
	Serial.println(can_read_register(0x2d), HEX);
*/

	return result;
}

boolean TrackController::receiveMessage(TrackMessage *message) {
	can_t can;

	bool result = dequeue(&can);

	if (result) {


//	boolean result = /* can_check_message() && */ can_get_message(&can);

//	if (result) {
        /*
		if (mDebug) {
			
			Serial.print("ID :");
			Serial.println(can.id, HEX);
			Serial.print("EXIDE:");
			Serial.println(can.flags.extended, HEX);
			Serial.print("DLC:");
			Serial.println(can.length, HEX);
			Serial.print("DATA:");

			for (int i = 0; i < can.length; i++) {
				printHex(can.data[i]);
			}
			
			Serial.println();
		}
        */
		message->command = (can.id >> 17) & 0xff;
		message->hash = can.id & 0xffff;
		message->response = bitRead(can.id, 16);
		message->length = can.length;

		for (int i = 0; i < can.length; i++) {
			message->data[i] = can.data[i];
		}

		if (mDebug) {
		    Serial.print("<== ");
		    printMessage(message);
		}
	}

	return result;
}

boolean TrackController::exchangeMessage(TrackMessage *message, word timeout) {
	ulong time = millis();

	byte command = message->command;
	
	if (!sendMessage(message)) {
		if (mDebug) {
			Serial.println("!!! Send error");
			Serial.println("!!! Emergency stop");
			setPower(false);
			for (;;);
		}
	}

	TrackMessage response;

	while (millis() < time + 5000) {
		clearMessage(&response);
		boolean result = receiveMessage(&response);

		if (result && response.command == command /* && message->response */) {
			return true;
		}
	}

	if (mDebug) {
		Serial.println("!!! Receive timeout");
	}
	
	return false;
}

void TrackController::clearMessage(TrackMessage *message) {
	message->command = 0;
	message->hash = 0;
	message->response = false;
	message->length = 0;
	for (int i = 0; i < 8; i++) {
		message->data[i] = 0;
	}
}

void TrackController::printMessage(TrackMessage *message) {
	printHex(message->command);
	
	Serial.print(message->response ? "R " : "  ");
	
	printHex(highByte(message->hash));
	printHex(lowByte(message->hash));
	printHex(message->length);

	for (int i = 0; i < message->length; i++) {
		printHex(message->data[i]);
	}
	
	Serial.println();
}

boolean TrackController::setPower(bool power) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x00;
	message.length = 0x05;
	message.data[4] = power ? 0x01 : 0x00;

	return exchangeMessage(&message, 1000);
}

boolean TrackController::setLocoDirection(word address, byte direction) {
	TrackMessage message;

	clearMessage(&message);
/*
	message.command = 0x00;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = 0x03;

	exchangeMessage(&message, 1000);
	
	clearMessage(&message);
*/	
	message.command = 0x05;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = direction;

	return exchangeMessage(&message, 1000);
}

boolean TrackController::toggleLocoDirection(word address) {
    return setLocoDirection(address, DIR_CHANGE);	
}

boolean TrackController::setLocoSpeed(word address, word speed) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x04;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(speed);
	message.data[5] = lowByte(speed);

	return exchangeMessage(&message, 1000);
}

boolean TrackController::accelerateLoco(word address) {
	word speed;
	
	if (getLocoSpeed(address, &speed)) {
		speed += 1024/14;
		if (speed > 1023) {
			speed = 1023;
		}
		
	    return setLocoSpeed(address, speed);
	}
	
	return false;
}

boolean TrackController::decelerateLoco(word address) {
	word speed;
	
	if (getLocoSpeed(address, &speed)) {
		speed -= 1024/14;
		if (speed > 32767) {
			speed = 0;
		}
		
	    return setLocoSpeed(address, speed);
	}
	
	return false;
}

boolean TrackController::setLocoFunction(word address, byte function, byte power) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x06;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = function;
	message.data[5] = power;

	return exchangeMessage(&message, 1000);
}

boolean TrackController::toggleLocoFunction(word address, byte function) {
    byte power;
    if (getLocoFunction(address, function, &power)) {
    	return setLocoFunction(address, function, power ? 0 : 1);
    }
    
    return false;
}

boolean TrackController::setAccessory(word address, byte position, byte power,
		word time) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x0b;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = position;
	message.data[5] = power;

	exchangeMessage(&message, 1000);

	if (time != 0) {
		delay(time);
		
		clearMessage(&message);

		message.command = 0x0b;
		message.length = 0x06;
		message.data[2] = highByte(address);
		message.data[3] = lowByte(address);
		message.data[4] = position;

		exchangeMessage(&message, 1000);
	}
	
	return true;
}

boolean TrackController::setTurnout(word address, boolean straight) {
	return setAccessory(address, straight ? ACC_STRAIGHT : ACC_ROUND, 1, 0);
}

boolean TrackController::getLocoDirection(word address, byte *direction) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x05;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(&message, 1000)) {
		direction[0] = message.data[4];
		return true;
	} else {
		false;
	}
}

boolean TrackController::getLocoSpeed(word address, word *speed) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x04;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(&message, 1000)) {
		speed[0] = word(message.data[4], message.data[5]);
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getLocoFunction(word address, byte function,
		byte *power) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x06;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = function;

	if (exchangeMessage(&message, 1000)) {
		power[0] = message.data[5];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getAccessory(word address, byte *position, byte *power) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x0b;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(&message, 1000)) {
		position[0] = message.data[4];
		power[0] = message.data[5];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::writeConfig(word address, word number, byte value) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x08;
	message.length = 0x08;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(number);
	message.data[5] = lowByte(number);
	message.data[6] = value;

	return exchangeMessage(&message, 5000);
}

boolean TrackController::readConfig(word address, word number, byte *value) {
	TrackMessage message;

	clearMessage(&message);

	message.command = 0x07;
	message.length = 0x07;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(number);
	message.data[5] = lowByte(number);
	message.data[6] = 0x01;

	if (exchangeMessage(&message, 5000)) {
		value[0] = message.data[6];
		return true;
	} else {
		return false;
	}
}

// ===================================================================
// === TrackControllerInfrared =======================================
// ===================================================================

#define ADDR_LOCO_1     0b11000
#define ADDR_LOCO_2     0b11001
#define ADDR_LOCO_3     0b11011
#define ADDR_LOCO_4     0b11100
#define ADDR_TURNOUT_A  0b01110
#define ADDR_TURNOUT_B  0b01111

#define CMD_FUNCTION  0b1010000
#define CMD_FASTER    0b0010000
#define CMD_SLOWER    0b0010001
#define CMD_DIRECTION 0b0001101
#define CMD_POWER_OFF 0b0001100
#define CMD_POWER_ON  0b0001110

static word locoBits[] = { ADDR_LOCO_1, ADDR_LOCO_2, ADDR_LOCO_3, ADDR_LOCO_4 };

TrackControllerInfrared::TrackControllerInfrared(bool debug) {
	mDebug = debug;
	
	mPower = true;
	
	for (int i = 0; i < 2; i++) {
		for (int j = 1; j <= 4; j++) {
			toggleLocoDirection(j);
		}
	}
	
	setPower(false);
}

boolean TrackControllerInfrared::sendMessage(word address, word command) {
	if (mPower) {
		int transmission = (address << 6) | (command & 0x3f);
		
		if (command >= 0x40) {
			sendRC5(mToggle | transmission, 12, true);
		} else {
			int command = (address << 6) | (command & 0x3f);
			sendRC5(mToggle | transmission, 12, false);
		}
		
		mToggle ^= 1 << 11;
		
		delay(50);
		
		return true;
	}
	
	return false;
}

boolean TrackControllerInfrared::accelerateLoco(int loco) {
	if (loco >= 1 && loco <= 4) {
		return sendMessage(locoBits[loco - 1], CMD_FASTER);
	}
	
	return false;
}

boolean TrackControllerInfrared::decelerateLoco(int loco) {
	if (loco >= 1 && loco <= 4) {
		return sendMessage(locoBits[loco - 1], CMD_SLOWER);
	}
	
	return false;
}

boolean TrackControllerInfrared::toggleLocoDirection(int loco) {
	if (loco >= 1 && loco <= 4) {
		return sendMessage(locoBits[loco - 1], CMD_DIRECTION);
	}
	
	return false;
}

boolean TrackControllerInfrared::toggleLocoFunction(int loco, int function) {
	if (loco >= 1 && loco <= 4 && function >= 0 && function <= 8) {
		return sendMessage(locoBits[loco - 1], CMD_FUNCTION | function);
	}
	
	return false;
}

boolean TrackControllerInfrared::setPower(boolean power) {
	if (mPower != power) {
		mPower = true;
		sendMessage(ADDR_LOCO_1, power ? CMD_POWER_ON : CMD_POWER_OFF);
		mPower = power;
		
		return true;
	}
	
	return false;
}

boolean TrackControllerInfrared::setTurnout(int turnout, boolean through) {
	if (turnout >= 1 && turnout <= 8) {
		word command = (turnout - 1) | (through ? 0x20 : 0x00);
		return sendMessage(ADDR_TURNOUT_A, command);
	} else if (turnout >= 9 && turnout <= 16) {
		word command = (turnout - 9) | (through ? 0x20 : 0x00);
		return sendMessage(ADDR_TURNOUT_B, command);
	}

	return false;
}

// ===================================================================
// === TrackReporter =================================================
// ===================================================================

const int DATA = A0;
const int CLOCK = 2;
const int LOAD = 3;
const int RESET = 4;

const int TIME = 50;

TrackReporter::TrackReporter(int size) {
	mSize = size;
	
	// pinMode(DATA, INPUT);
	pinMode(CLOCK, OUTPUT);
	pinMode(LOAD, OUTPUT);
	pinMode(RESET, OUTPUT);
}

void TrackReporter::refresh() {
	int myByte = 0;
	int myBit = 0;

	for (int i = 0; i <= sizeof(mSwitches); i++) {
		mSwitches[i] = 0;
	}

	digitalWrite(LOAD, HIGH);
	delayMicroseconds( TIME);
	digitalWrite(CLOCK, HIGH);
	delayMicroseconds(TIME);
	digitalWrite(CLOCK, LOW);
	delayMicroseconds(TIME);
	digitalWrite(RESET, HIGH);
	delayMicroseconds(TIME);
	digitalWrite(RESET, LOW);
	delayMicroseconds(TIME);
	digitalWrite(LOAD, LOW);

	delayMicroseconds(TIME / 2);

	bitWrite(mSwitches[myByte], myBit++, digitalRead(DATA));
	delayMicroseconds(TIME / 2);

	for (int i = 1; i < mSize; i++) {
		digitalWrite(CLOCK, HIGH);
		delayMicroseconds(TIME);
		digitalWrite(CLOCK, LOW);

		delayMicroseconds(TIME / 2);
		bitWrite(mSwitches[myByte], myBit++, digitalRead(DATA));

		if (myBit == 8) {
			myByte++;
			myBit = 0;
		}

		delayMicroseconds(TIME / 2);
	}
}

boolean TrackReporter::getValue(int index) {
	index--;
	return bitRead(mSwitches[index / 8], index % 8);
}
