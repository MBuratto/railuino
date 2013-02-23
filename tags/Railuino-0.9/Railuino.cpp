/*********************************************************************
 * Railuino - Hacking your Märklin
 *
 * Copyright (C) 2012 Joerg Pleumann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LICENSE file for more details.
 */

// #include <can.h>
#include "Railuino.h"
// #include "canbus/canbus.c"
#include "can/mcp2515.h"
#include "can/mcp2515.c"

#if defined(__LEONARDO__)
#include "ir/infrared2.c"
#else
#include "ir/infrared.c"
#endif

size_t printHex(Print &p, unsigned long hex, int digits) {
    size_t size = 0;

    String s = String(hex, HEX);

    for (int i = s.length(); i < digits; i++) {
        size += p.print("0");
    }

    size += p.print(s);

    return size;
}

int parseHex(String &s, int start, int end, boolean *ok) {
    int value = 0;

    for (int i = start; i < end; i++) {
    	char c = s.charAt(i);

        if (c >= '0' && c <= '9') {
            value = 16 * value + c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value = 16 * value + 10 + c - 'a';
        } else if (c >= 'A' && c <= 'F') {
            value = 16 * value + 10 + c - 'A';
        } else {
        	ok = false;
            return -1;
        }
    }

    return value;
}

#define SIZE 32

#define ulong unsigned long

can_t _buffer[SIZE];

volatile int posRead = 0;

volatile int posWrite = 0;

volatile boolean lastOpWasWrite = false;

void enqueue() {
	if (posWrite == posRead && lastOpWasWrite) {
		// Serial.println("!!! Buffer full");
		return;
	}

	if (can_get_message(&_buffer[posWrite])) {
		posWrite = (posWrite + 1) % SIZE;
	} else {
		// Serial.println("!!! No message");
	}

	lastOpWasWrite = true;
}

boolean dequeue(can_t *p) {
	noInterrupts();

	if (posWrite == posRead && !lastOpWasWrite) {
		interrupts();
		return false;
	}

	memcpy(p, &_buffer[posRead], sizeof(can_t));
/*
	p->id=_buffer[posRead].id;
	p->length=_buffer[posRead].length;

	for (int i = 0; i < p->length; i++) {
		p->data[i] = _buffer[posRead].data[i];
	}
*/
	//*p = _buffer[posRead];

	posRead = (posRead + 1) % SIZE;
	lastOpWasWrite = false;

	interrupts();

	return true;
}

// ===================================================================
// === TrackMessage ==================================================
// ===================================================================

void TrackMessage::clear() {
	command = 0;
	hash = 0;
	response = false;
	length = 0;
	for (int i = 0; i < 8; i++) {
		data[i] = 0;
	}
}

size_t TrackMessage::printTo(Print& p) const {
    size_t size = 0;

    size += printHex(p, hash, 4);
    size += p.print(response ? " R " : "   ");
    size += printHex(p, command, 2);
    size += p.print(" ");
    size += printHex(p, length, 1);

    for (int i = 0; i < length; i++) {
        size += p.print(" ");
        size += printHex(p, data[i], 2);
    }

    return size;
}

boolean TrackMessage::parseFrom(String &s) {
	boolean result = true;

	clear();

	if (s.length() < 11) {
		return false;
	}

	hash = parseHex(s, 0, 4, &result);
	response = s.charAt(5) != ' ';
	command = parseHex(s, 7, 9, &result);
	length = parseHex(s, 10, 11, &result);

	if (length > 8) {
		return false;
	}

	if (s.length() < 11 + 3 * length) {
		return false;
	}

	for (int i = 0; i < length; i++) {
		data[i] = parseHex(s, 12 + 3 * i, 12 + 3 * i + 2, &result);
	}

	return result;
}

// ===================================================================
// === TrackController ===============================================
// ===================================================================

TrackController::TrackController() {
	if (mDebug) {
		Serial.println(F("### Creating controller"));
	}

	init(0, false, false);
}

TrackController::TrackController(word hash, boolean debug) {
	if (mDebug) {
		Serial.println(F("### Creating controller"));
	}

	init(hash, debug, false);
}

TrackController::~TrackController() {
	if (mDebug) {
		Serial.println(F("### Destroying controller"));
	}

	end();
}

void TrackController::init(word hash, boolean debug, boolean loopback) {
	mHash = hash;
	mDebug = debug;
	mLoopback = loopback;
}

word TrackController::getHash() {
	return mHash;
}

boolean TrackController::isDebug() {
	return mDebug;
}

boolean TrackController::isLoopback() {
	return mLoopback;
}

void TrackController::begin() {
	attachInterrupt(CAN_INT, enqueue, LOW);

	if (!can_init(5, mLoopback)) {
		Serial.println(F("!!! Init error"));
		Serial.println(F("!!! Emergency stop"));
		for (;;);
	}

	delay(500);

	if (!mLoopback) {
		TrackMessage message;

		message.clear();
		message.command = 0x1b;
		message.length = 0x05;
		message.data[4] = 0x11;

		sendMessage(message);
	}

	if (mHash == 0) {
		generateHash();
	}

}

void TrackController::generateHash() {
	TrackMessage message;

	boolean ok = false;

	while(!ok) {
		mHash = random(0x10000) & 0xff7f | 0x0300;

		if (mDebug) {
			Serial.print(F("### Trying new hash "));
			printHex(Serial, mHash, 4);
			Serial.println();
		}

		message.clear();
		message.command = 0x18;

		sendMessage(message);

		delay(500);

		ok = true;
		while(receiveMessage(message)) {
			if (message.hash == mHash) {
				ok = false;
			}
		}
	}

	if (mDebug) {
        Serial.println(F("### New hash looks good"));
	}
}

// end - no interrupts

void TrackController::end() {
	detachInterrupt(CAN_INT);

	can_t t;

	boolean b = dequeue(&t);
	while (b) {
		b = dequeue(&t);
	}
}

boolean TrackController::sendMessage(TrackMessage &message) {
	can_t can;

	message.hash = mHash;
	
	can.id = ((uint32_t)message.command) << 17 | (uint32_t)message.hash;
	can.flags.extended = 1;
	can.flags.rtr = 0;
	can.length = message.length;

	for (int i = 0; i < message.length; i++) {
		can.data[i] = message.data[i];
	}

	if (mDebug) {
	    Serial.print("==> ");
	    Serial.println(message);
	}
	
	return can_send_message(&can);
}

boolean TrackController::receiveMessage(TrackMessage &message) {
	can_t can;

	boolean result = dequeue(&can);
//	boolean result = /* can_check_message() && */ can_get_message(&can);

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
		message.clear();
		message.command = (can.id >> 17) & 0xff;
		message.hash = can.id & 0xffff;
		message.response = bitRead(can.id, 16) || mLoopback;
		message.length = can.length;

		for (int i = 0; i < can.length; i++) {
			message.data[i] = can.data[i];
		}

		if (mDebug) {
		    Serial.print("<== ");
		    Serial.println(message);
		}
	}

	return result;
}

boolean TrackController::exchangeMessage(TrackMessage &out, TrackMessage &in, word timeout) {
	int command = out.command;

	if (!sendMessage(out)) {
		if (mDebug) {
			Serial.println(F("!!! Send error"));
			Serial.println(F("!!! Emergency stop"));
			setPower(false);
			for (;;);
		}
	}

	ulong time = millis();

	// TrackMessage response;

	while (millis() < time + timeout) {
		in.clear();
		boolean result = receiveMessage(in);

		if (result && in.command == command && in.response) {
			return true;
		}
	}

	if (mDebug) {
		Serial.println(F("!!! Receive timeout"));
	}
	
	return false;
}

boolean TrackController::setPower(boolean power) {
	TrackMessage message;

	if (power) {
		message.clear();
		message.command = 0x00;
		message.length = 0x07;
		message.data[4] = 9;
		message.data[6] = 0xD;

		exchangeMessage(message, message, 1000);

		message.clear();
		message.command = 0x00;
		message.length = 0x06;
		message.data[4] = 8;
		message.data[5] = 7;

		exchangeMessage(message, message, 1000);
	}

	message.clear();
	message.command = 0x00;
	message.length = 0x05;
	message.data[4] = power ? 0x01 : 0x00;

	return exchangeMessage(message, message, 1000);
}

boolean TrackController::setLocoDirection(word address, byte direction) {
	TrackMessage message;

    message.clear();
	message.command = 0x00;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = 0x03;

	exchangeMessage(message, message, 1000);
	
	message.clear();
	message.command = 0x05;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = direction;

	return exchangeMessage(message, message, 1000);
}

boolean TrackController::toggleLocoDirection(word address) {
    return setLocoDirection(address, DIR_CHANGE);	
}

boolean TrackController::setLocoSpeed(word address, word speed) {
	TrackMessage message;

	message.clear();
	message.command = 0x04;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(speed);
	message.data[5] = lowByte(speed);

	return exchangeMessage(message, message, 1000);
}

boolean TrackController::accelerateLoco(word address) {
	word speed;
	
	if (getLocoSpeed(address, &speed)) {
		speed += 77;
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
		speed -= 77;
		if (speed > 32767) {
			speed = 0;
		}
		
	    return setLocoSpeed(address, speed);
	}
	
	return false;
}

boolean TrackController::setLocoFunction(word address, byte function, byte power) {
	TrackMessage message;

	message.clear();
	message.command = 0x06;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = function;
	message.data[5] = power;

	return exchangeMessage(message, message, 1000);
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

	message.clear();
	message.command = 0x0b;
	message.length = 0x06;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = position;
	message.data[5] = power;

	exchangeMessage(message, message, 1000);

	if (time != 0) {
		delay(time);
		
		message.clear();
		message.command = 0x0b;
		message.length = 0x06;
		message.data[2] = highByte(address);
		message.data[3] = lowByte(address);
		message.data[4] = position;

		exchangeMessage(message, message, 1000);
	}
	
	return true;
}

boolean TrackController::setTurnout(word address, boolean straight) {
	return setAccessory(address, straight ? ACC_STRAIGHT : ACC_ROUND, 1, 1000);
}

boolean TrackController::getLocoDirection(word address, byte *direction) {
	TrackMessage message;

	message.clear();
	message.command = 0x05;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(message, message, 1000)) {
		direction[0] = message.data[4];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getLocoSpeed(word address, word *speed) {
	TrackMessage message;

	message.clear();
	message.command = 0x04;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(message, message, 1000)) {
		speed[0] = word(message.data[4], message.data[5]);
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getLocoFunction(word address, byte function,
		byte *power) {
	TrackMessage message;

	message.clear();
	message.command = 0x06;
	message.length = 0x05;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = function;

	if (exchangeMessage(message, message, 1000)) {
		power[0] = message.data[5];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getAccessory(word address, byte *position, byte *power) {
	TrackMessage message;

	message.clear();
	message.command = 0x0b;
	message.length = 0x04;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);

	if (exchangeMessage(message, message, 1000)) {
		position[0] = message.data[4];
		power[0] = message.data[5];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::writeConfig(word address, word number, byte value) {
	TrackMessage message;

	message.clear();
	message.command = 0x08;
	message.length = 0x08;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(number);
	message.data[5] = lowByte(number);
	message.data[6] = value;

	return exchangeMessage(message, message, 10000);
}

boolean TrackController::readConfig(word address, word number, byte *value) {
	TrackMessage message;

	message.clear();
	message.command = 0x07;
	message.length = 0x07;
	message.data[2] = highByte(address);
	message.data[3] = lowByte(address);
	message.data[4] = highByte(number);
	message.data[5] = lowByte(number);
	message.data[6] = 0x01;

	if (exchangeMessage(message, message, 10000)) {
		value[0] = message.data[6];
		return true;
	} else {
		return false;
	}
}

boolean TrackController::getVersion(byte *high, byte *low) {
    boolean result = false;

    TrackMessage message;
    
    message.clear();
    message.command = 0x18;

    sendMessage(message);

    delay(500);

    while(receiveMessage(message)) {
        if (message.command = 0x18 && message.data[6] == 0x00 && message.data[7] == 0x10) {
            (*high) = message.data[4];
            (*low) = message.data[5];
            result = true;
        }
    }
    
    return result;
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

TrackControllerInfrared::TrackControllerInfrared() {
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
// === TrackReporterS88 ==============================================
// ===================================================================

const int DATA = A0;
const int CLOCK = 2;
const int LOAD = 3;
const int RESET = 4;

const int TIME = 50;

TrackReporterS88::TrackReporterS88(int modules) {
	mSize = modules;
	
	// pinMode(DATA, INPUT);
	pinMode(CLOCK, OUTPUT);
	pinMode(LOAD, OUTPUT);
	pinMode(RESET, OUTPUT);
}

void TrackReporterS88::refresh() {
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

	for (int i = 1; i < 16 * mSize; i++) {
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

boolean TrackReporterS88::getValue(int index) {
	index--;
	return bitRead(mSwitches[index / 8], index % 8);
}

// ===================================================================
// === TrackReporterIOX ==============================================
// ===================================================================


void SPI_begin() {
  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);

  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);

  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
}

byte SPI_transfer(byte _data) {
  SPDR = _data;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;
}

byte ioxCount;

byte ioxSwitches[16];

byte ioxSwitches2[16];

unsigned int readRegister(byte address, byte index) {
  digitalWrite(6, LOW);

  SPI_transfer(65 | (address << 1));
  SPI_transfer(index);
  unsigned int result = SPI_transfer(255);

  digitalWrite(6, HIGH);

  return result;
}

void writeRegister(byte address, byte index, byte value) {
  digitalWrite(6, LOW);

  SPI_transfer(64 | (address << 1));
  SPI_transfer(index);
  SPI_transfer(value);

  digitalWrite(6, HIGH);
}

void handleInterrupt0() {
  noInterrupts();

  for (int i = 0; i < ioxCount; i++) {
    ioxSwitches[2 * i] = readRegister(i, 9);
    ioxSwitches[2 * i + 1] = readRegister(16 + i, 9);
    
    ioxSwitches2[2 * i] |= ioxSwitches[2 * i];
    ioxSwitches2[2 * i + 1] |= ioxSwitches[2 * i + 1];
  }

  interrupts();
}

TrackReporterIOX::TrackReporterIOX(int modules) {
  mCount = modules;

  ioxCount = modules;

  digitalWrite(6, HIGH);
  pinMode(6, OUTPUT);
  
  SPI_begin();
  
  noInterrupts();
  for (int i = 0; i < mCount; i++) {
    writeRegister(i, 5, 76);   // Open drain, banks in case of 16 bit

    for (int j = 0; j <= 16; j += 16) {
      writeRegister(i, j + 0, 255); // IODIR: All GPIOs are inputs
      writeRegister(i, j + 1, 255); // IOPOL: GND means locial 1
      writeRegister(i, j + 2, 255); // GPINTEN: All interrupts enabled 
      writeRegister(i, j + 3, 0);   // DEFVAL: Compare default value
      //writeRegister(i, j + 4, 255); // INTCON: Compare against default value
      writeRegister(i, j + 6, 255); // Pull-up resistors
      readRegister(i, j + 9);
    }
  }

  attachInterrupt(1, &handleInterrupt0, LOW);
  interrupts();
}

TrackReporterIOX::~TrackReporterIOX() {
  detachInterrupt(1);
}

void TrackReporterIOX::refresh() {
  noInterrupts();

  for (int i = 0; i < mCount; i++) {
    mSwitches[i] = ioxSwitches2[i];
    ioxSwitches2[i] = ioxSwitches[i];
  }

  interrupts();
}

boolean TrackReporterIOX::getValue(int index) {
  index--;
  return bitRead(mSwitches[index / 8], index % 8);
}
