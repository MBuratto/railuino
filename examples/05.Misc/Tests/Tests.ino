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

#include <Railuino.h>

// Controls whether all messages are being shown
#define DEBUG true

// Controls whether stress tests are included
#define STRESS true

// Locomotive that is being used during the tests (DCC required for reading CVs)
#define LOCO ADDR_MM2 + 78

// Turnout that is being used during the tests
#define TURN ADDR_ACC_MM2 + 1

// Signal that is being used during the tests
#define SIGN ADDR_ACC_MM2 + 2

// Number of passed and failed tests
int pass = 0;
int fail = 0;

// Assertion facility, macros and helper functions
#define TEST __TEST__(__FUNCTION__);
#define PASS pass++; return;
#define FAIL fail++; return;
#define ASSERT(x, y) if(!__ASSERT__(x, y, __FUNCTION__)) { FAIL; }

void __TEST__(const char funcName[]) {
  Serial.println();
  Serial.print("***** ");
  Serial.print(funcName);
  Serial.println(" *****");
  Serial.println();
}

boolean __ASSERT__(int assertNo, boolean exprValue, const char funcName[]) {
  if (!(exprValue)) {
    Serial.print(F("Assertion "));
    Serial.print(assertNo);
    Serial.print(F(" in function "));
    Serial.print(funcName);
    Serial.println(F("() failed"));
    
    return false;
  }
  
  return true;
}

// One-time initializer actually does all the work
void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print(F("Press <Return> to start Railuino "));
  Serial.print(RAILUINO_VERSION >> 8);
  Serial.print(F("."));
  Serial.print(RAILUINO_VERSION & 0xff);
  Serial.println(F(" test suite..."));
  while(true) {
    int c = Serial.read();
    if (c == 10 || c == 13) {
      break;
    }
  }
  
  testMessageClear();
  testMessagePrintTo();
  testMessageParseFrom();
  
  testController();
  testInitController();
  testBeginEnd();
  testSendReceiveMessage();
  testExchangeMessage();
  
  testVersion();
  testPower();
  testGetSetDirection();
  testToggleDirection();
  testGetSetSpeed();
  testAccelerate();
  testDecelerate();
  testGetSetFunction();
  testToggleFunction();
  testSetAccessory();
  testSetTurnout();
  testReadWriteConfig();

  if (STRESS) {
    testSendReceiveMessageStress1();
    testSendReceiveMessageStress2();
    testExchangeMessageStress();
  }
  
  Serial.println();
  Serial.print(F("Total tests  : ")); Serial.println(pass + fail);
  Serial.print(F("Passed tests : ")); Serial.println(pass);
  Serial.print(F("Failed tests : ")); Serial.println(fail);
  Serial.println();
  Serial.println(fail == 0 ? ":-)" : ":-(");  
  Serial.flush();
}

// Loop stays empty
void loop() {
}

// Tests clearing a message
void testMessageClear() {
  TEST;

  TrackMessage message;
  
  message.command  = 0xff;
  message.response = true;
  message.hash     = 0xffff;
  message.length   = 0x08;
  
  for (int i = 0; i < 8; i++) {
    message.data[i] = 0xff;
  }
  
  message.clear();
  
  ASSERT(0, message.command == 0);
  ASSERT(1, message.response == false);
  ASSERT(2, message.hash == 0);
  ASSERT(3, message.length == 0);
  
  for (int i = 0; i < 8; i++) {
    ASSERT(4 + i, message.data[i] == 0);
  }
  
  PASS;
}

// Tests printing a message to a Serial
void testMessagePrintTo() {
  TEST;

  TrackMessage message;
  
  message.command  = 0xff;
  message.response = true;
  message.hash     = 0xdead;
  message.length   = 0x08;
  
  for (int i = 0; i < 8; i++) {
    message.data[i] = i;
  }

  size_t bytesPrinted = Serial.println(message);
  
  ASSERT(0, bytesPrinted == 37);
  
  PASS;
}

// Tests parsing a message from a String 
void testMessageParseFrom() {
  TEST;

  TrackMessage message;
  
  String good1 = "dead   ff 8 00 01 02 03 04 05 06 07";
  String good2 = "babe R 00 0";
  String good3 = "abba R 88 2 00 01 02 03 IGNORE THIS";
  
  String bad1 = "";
  String bad2 = "cafe   00 8";
  String bad3 = "dada   00 9 00 01 02 03 04 05 06 07 08";
  String bad4 = "The quick brown fox jumped over the lazy dog.";
  
  ASSERT(0, message.parseFrom(good1));
  
  ASSERT(1, message.command  == 0xff);
  ASSERT(2, message.response == false);
  ASSERT(3, message.hash     == 0xdead);
  ASSERT(4, message.length   == 0x08);

  for (int i = 0; i < 8; i++) {
    ASSERT(5 + i, message.data[i] == i);
  }

  ASSERT(13, message.parseFrom(good2));
  
  ASSERT(14, message.command  == 0x00);
  ASSERT(15, message.response == true);
  ASSERT(16, message.hash     == 0xbabe);
  ASSERT(17, message.length   == 0x00);

  ASSERT(18, message.parseFrom(good3));
  
  ASSERT(19, message.command  == 0x88);
  ASSERT(20, message.response == true);
  ASSERT(21, message.hash     == 0xabba);
  ASSERT(21, message.length   == 0x02);

  ASSERT(22, message.data[0] == 0x00);
  ASSERT(23, message.data[1] == 0x01);
  
  for (int i = 2; i < 8; i++) {
    ASSERT(24 + i - 2, message.data[i] == 0);
  }

  ASSERT(30, !message.parseFrom(bad1));
  ASSERT(31, !message.parseFrom(bad2));
  ASSERT(32, !message.parseFrom(bad3));
  ASSERT(33, !message.parseFrom(bad4));
  
  PASS;
}

// Tests creating the controller
void testController() {
  TEST;

  TrackController ctrl1;

  ASSERT(0, ctrl1.getHash() == 0);
  ASSERT(1, !ctrl1.isDebug());
  ASSERT(2, !ctrl1.isLoopback());
  
  TrackController ctrl2(0x1234, false);

  ASSERT(3, ctrl2.getHash() == 0x1234);
  ASSERT(4, !ctrl2.isDebug());
  ASSERT(5, !ctrl2.isLoopback());

  TrackController ctrl3(0x1234, true);
  
  ASSERT(6, ctrl3.getHash() == 0x1234);
  ASSERT(7, ctrl3.isDebug());
  ASSERT(8, !ctrl3.isLoopback());
  
  PASS;
}

// Tests initializing the controller
void testInitController() {
  TEST;

  TrackController ctrl;
  
  ctrl.init(0x1234, false, false); // Must not use DEBUG constant!
  ASSERT(0, ctrl.getHash() == 0x1234);
  ASSERT(1, !ctrl.isDebug());
  ASSERT(2, !ctrl.isLoopback());

  ctrl.init(0x5678, true, true); // Must not use DEBUG constant!
  ASSERT(3, ctrl.getHash() == 0x5678);
  ASSERT(4, ctrl.isDebug());
  ASSERT(5, ctrl.isLoopback());
  
  PASS;
}

// Tests controller start/stop including hash generation
void testBeginEnd() {
  TEST;

  TrackController ctrl;
  
  ctrl.init(0x7f7f, DEBUG, false);
  ctrl.begin();
  ASSERT(0, ctrl.getHash() == 0x7f7f);
  ctrl.end();

  ctrl.init(0, DEBUG, false);
  ctrl.begin();
  ASSERT(1, ctrl.getHash() != 0);
  ASSERT(2, ctrl.getHash() & 0x0300 == 0x0300);
  ASSERT(3, ctrl.getHash() | 0xff7f == 0xff7f);
  ctrl.end();
  
  PASS;
}

// Tests sending and receiving messages
void testSendReceiveMessage() {
  TEST;

  TrackController ctrl;
  TrackMessage out;
  TrackMessage in;
  
  String text = "dead   ff 8 00 01 02 03 04 05 06 07";
  
  ctrl.init(0x7f7f, DEBUG, true);
  ctrl.begin();
  
  out.parseFrom(text);

  // No response without a sent message
  ASSERT(0, !ctrl.receiveMessage(in));
  
  // Single message is reflected
  ASSERT(1, ctrl.sendMessage(out));
  
  delay(20);
  
  ASSERT(2, ctrl.receiveMessage(in));
  ASSERT(3, in.hash == out.hash);
  ASSERT(4, in.response);
  ASSERT(5, in.command == out.command);
  ASSERT(6, in.length == out.length);
  
  for (int i = 0; i < in.length; i++) {
    ASSERT(7 + i, in.data[i] == out.data[i]);
  }

  ASSERT(19, !ctrl.receiveMessage(in));

  PASS;
}

// Tests sending/receiving many messages one-by-one  
void testSendReceiveMessageStress1() {
  TEST;

  TrackController ctrl;
  TrackMessage out;
  TrackMessage in;
  
  ctrl.init(0x7f7f, DEBUG, true);
  ctrl.begin();
  
  // Bunch of send/receive calls 
  for (int i = 0; i < 1000; i++) {
    int a = 20 * i;
    
    out.clear();
    out.hash = i;
    out.command = random(256);
    out.length = random(9);
  
    for (int j = 0; j < out.length; j++) {
      out.data[j] = random(256);
    }
  
    ASSERT(a + 0, !ctrl.receiveMessage(in));
    ASSERT(a + 1, ctrl.sendMessage(out));
    
    delay(20);
    
    ASSERT(a + 2, ctrl.receiveMessage(in));
  
    ASSERT(a + 3, in.response);
   
    ASSERT(a + 4, in.hash == out.hash);
    ASSERT(a + 5, in.command == out.command);
    ASSERT(a + 6, in.length == out.length);
  
    for (int k = 0; k < in.length; k++) {
      ASSERT(a + 10 + k, in.data[k] == out.data[k]);
    }
  }

  PASS;
}

// Tests sending/receiving many messages in bulk
void testSendReceiveMessageStress2() {
  TEST;

  TrackController ctrl;
  TrackMessage out;
  TrackMessage in;
  
  ctrl.init(0x7f7f, DEBUG, true);
  ctrl.begin();

  int a = 0;

  // Bunch of "fill buffer, then empty it"
  for (int i = 0; i < 32; i++) {
    ASSERT(a++, !ctrl.receiveMessage(in));
    
    for (int j = 0; j < 32; j++) {
      out.clear();
      out.command = j;
      out.length = j % 9;
    
      for (int k = 0; k < out.length; k++) {
        out.data[k] = k;
      }
    
      ASSERT(a++, ctrl.sendMessage(out));
      
      delay(10);
    }
    
    for (int j = 0; j < 32; j++) {
      ASSERT(a++, ctrl.receiveMessage(in));
    
      ASSERT(a++, in.response);
     
      ASSERT(a++, in.hash == 0x7f7f);
      ASSERT(a++, in.command == j);
      ASSERT(a++, in.length == j % 9);
    
      for (int k = 0; k < in.length; k++) {
        ASSERT(a++, in.data[k] == k);
      }
    }
  }

  PASS;
}

// Tests exchanging messages
void testExchangeMessage() {
  TEST;

  TrackController ctrl;
  TrackMessage out;
  TrackMessage in;
  
  String text = "dead   ff 8 00 01 02 03 04 05 06 07";

  ctrl.init(0x7f7f, DEBUG, true);
  ctrl.begin();

  ASSERT(0, !ctrl.receiveMessage(in));
  
  // Single message is reflected
  out.parseFrom(text);
  
  ASSERT(1, ctrl.exchangeMessage(out, out, 1000));  
  ASSERT(2, out.response);
  ASSERT(3, out.command == 0xff);
  ASSERT(4, out.length == 8);
  
  for (int i = 0; i < out.length; i++) {
    ASSERT(5 + i, out.data[i] == i);
  }

  ASSERT(19, !ctrl.receiveMessage(in));

  // When using separate 'in' message 'out' stays untouched
  out.parseFrom(text);
  
  ASSERT(1, ctrl.exchangeMessage(out, in, 1000));  
  ASSERT(2, !out.response);
  ASSERT(3, in.response);
  ASSERT(4, in.command == 0xff);
  ASSERT(5, in.length == 8);
  
  for (int i = 0; i < in.length; i++) {
    ASSERT(6 + i, in.data[i] == i);
  }

  ASSERT(19, !ctrl.receiveMessage(in));

  // Non-matching responses are consumed
  out.command = 0x00;
  ASSERT(20, ctrl.sendMessage(out));
  
  out.parseFrom(text);

  ASSERT(21, ctrl.exchangeMessage(out, in, 1000));  
  ASSERT(22, in.response);
  ASSERT(23, in.command == 0xff);
  ASSERT(24, in.length == 8);
  
  for (int i = 0; i < in.length; i++) {
    ASSERT(25 + i, in.data[i] == i);
  }
  
  ASSERT(29, !ctrl.receiveMessage(in));
  
  PASS;
}

// Tests exchanging many messages one-by-one 
void testExchangeMessageStress() {
  TEST;

  TrackController ctrl;
  TrackMessage out;
  TrackMessage in;
  
  ctrl.init(0x7f7f, DEBUG, true);
  ctrl.begin();
  
  // Bunch of execute calls 
  for (int i = 0; i < 1000; i++) {
    int a = 20 * i;
    
    ASSERT(a + 0, !ctrl.receiveMessage(in));
    
    out.clear();
    out.command = random(256);
    out.length = random(9);
  
    for (int j = 0; j < out.length; j++) {
      out.data[j] = random(256);
    }
  
    ASSERT(a + 1, ctrl.exchangeMessage(out, in, 1000));
  
    ASSERT(a + 2, in.response);
   
    ASSERT(a + 3, in.hash == 0x7f7f);
    ASSERT(a + 4, in.command == out.command);
    ASSERT(a + 5, in.length == out.length);
  
    for (int k = 0; k < in.length; k++) {
      ASSERT(a + 10 + k, in.data[k] == out.data[k]);
    }
  }
  
  PASS;
}

// Tests the version
void testVersion() {
  TEST;

  TrackController ctrl;

  ctrl.init(0, DEBUG, false);  
  ctrl.begin();
  
  byte high, low;
  
  ASSERT(0, ctrl.getVersion(&high, &low));
  
  Serial.print("### Trackbox SW version is ");
  Serial.print(high, DEC);
  Serial.print(".");
  Serial.println(low, DEC);
  
  ASSERT(1, (word)(high << 8 | low) >= (word)TRACKBOX_VERSION);
  
  ctrl.end();
  
  PASS;
}

// Tests controlling the power
void testPower() {
  TEST;

  TrackController ctrl;

  ctrl.init(0, DEBUG, false);  
  ctrl.begin();
  
  ASSERT(0, ctrl.setPower(true));

  delay(1000);

  ASSERT(1, ctrl.setPower(false));

  delay(1000);
  
  ctrl.end();
  
  PASS;
}

// Tests getting/setting the direction of a loco
void testGetSetDirection() {
  TEST;

  TrackController ctrl;
  
  ctrl.init(0, DEBUG, false);  
  ctrl.begin();
  
  byte dir = 0;
  
  ASSERT(0, ctrl.setLocoDirection(LOCO, DIR_FORWARD));
  ASSERT(1, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(2, dir == DIR_FORWARD);
  
  delay(500);

  ASSERT(3, ctrl.setLocoDirection(LOCO, DIR_REVERSE));
  ASSERT(4, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(5, dir == DIR_REVERSE);

  delay(500);

  ASSERT(6, ctrl.setLocoDirection(LOCO, DIR_CHANGE));
  ASSERT(7, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(8, dir == DIR_FORWARD);
  
  delay(500);

  ASSERT(9, ctrl.setLocoDirection(LOCO, DIR_CURRENT));
  ASSERT(10, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(11, dir == DIR_FORWARD);

  delay(500);

  ctrl.end();
  
  PASS;
}

// Tests toggling the direction of a loco
void testToggleDirection() {
  TEST;

  TrackController ctrl;
  
  ctrl.init(0, DEBUG, false);  
  ctrl.begin();
  
  byte dir = 0;
  
  ASSERT(0, ctrl.setLocoDirection(LOCO, DIR_FORWARD));  

  delay(500);

  ASSERT(1, ctrl.toggleLocoDirection(LOCO));
  ASSERT(2, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(3, dir == DIR_REVERSE);

  delay(500);

  ASSERT(4, ctrl.toggleLocoDirection(LOCO));
  ASSERT(5, ctrl.getLocoDirection(LOCO, &dir));
  ASSERT(6, dir == DIR_FORWARD);

  delay(500);

  ctrl.end();
  
  PASS;
}

// Tests getting/setting the speed of a loco
void testGetSetSpeed() {
  TEST;

  TrackController ctrl;

  word speed = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();

  ASSERT(0, ctrl.setLocoSpeed(LOCO, 0));
  ASSERT(1, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(2, speed == 0);

  delay(500);

  ASSERT(3, ctrl.setLocoSpeed(LOCO, 500));
  ASSERT(4, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(5, speed == 500);

  delay(500);


  ASSERT(6, ctrl.setLocoSpeed(LOCO, 1000));
  ASSERT(7, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(8, speed == 1000);

  delay(500);

  ASSERT(9, ctrl.setLocoSpeed(LOCO, 1001));
  ASSERT(10, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(11, speed == 1000);
  
  delay(500);

  PASS;
}

// Tests accelerating a loco
void testAccelerate() {
  TEST;

  TrackController ctrl;

  word speed = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();

  ASSERT(0, ctrl.setLocoSpeed(LOCO, 0));
  ctrl.accelerateLoco(LOCO);
  ASSERT(1, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(2, speed == 77);

  delay(500);
  
  ASSERT(3, ctrl.setLocoSpeed(LOCO, 999));
  ctrl.accelerateLoco(LOCO);
  ASSERT(4, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(5, speed == 1000);
  
  delay(500);

  ctrl.end();
  
  PASS;
}

// Tests decelerating a loco
void testDecelerate() {
  TEST;

  TrackController ctrl;

  word speed = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();

  ASSERT(0, ctrl.setLocoSpeed(LOCO, 1000));
  ctrl.decelerateLoco(LOCO);
  ASSERT(1, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(2, speed == 923);

  delay(500);

  ASSERT(3, ctrl.setLocoSpeed(LOCO, 1));
  ctrl.decelerateLoco(LOCO);
  ASSERT(4, ctrl.getLocoSpeed(LOCO, &speed));
  ASSERT(5, speed == 0);
  
  delay(500);
  
  ctrl.end();
  
  PASS;
}

// Tests getting/setting the functions of a loco
void testGetSetFunction() {
  TEST;

  TrackController ctrl;

  byte value = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();

  for (int i = 0; i < 4; i++) {
    ASSERT(0, ctrl.setLocoFunction(LOCO, i, 1));
    ASSERT(1, ctrl.getLocoFunction(LOCO, i, &value));
    ASSERT(2, value != 0);
    
    delay(500);
    
    ASSERT(3, ctrl.setLocoFunction(LOCO, i, 0));
    ASSERT(4, ctrl.getLocoFunction(LOCO, i, &value));
    ASSERT(5, value == 0);
    
    delay(500);
  }
  
  ctrl.end();
  
  PASS;
}

// Tests toggling the functions of a loco
void testToggleFunction() {
  TEST;

  TrackController ctrl;

  byte value = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();

  for (int i = 0; i < 4; i++) {
    ASSERT(0, ctrl.setLocoFunction(LOCO, i, 0));
    
    ASSERT(1, ctrl.toggleLocoFunction(LOCO, i));
    ASSERT(2, ctrl.getLocoFunction(LOCO, i, &value));
    ASSERT(3, value != 0);

    delay(500);
        
    ASSERT(4, ctrl.toggleLocoFunction(LOCO, i));
    ASSERT(5, ctrl.getLocoFunction(LOCO, i, &value));
    ASSERT(6, value == 0);
    
    delay(500);
  }
  
  ctrl.end();
  
  PASS;
}

// Tests controlling a (magnetic) accessory
void testSetAccessory() {
  TEST;

  TrackController ctrl;

  byte state = 0;
  byte power = 0;
  
  ctrl.init(0, DEBUG, false);
  ctrl.begin();
  ctrl.setLocoSpeed(LOCO, 0);
  ctrl.setPower(true);
  
  byte states[4] = { ACC_HP0, ACC_HP1, ACC_HP2, ACC_SH0 };
  
  for (int i = 0 ; i < 4; i++) {
    ASSERT(0, ctrl.setAccessory(SIGN, states[i], 1, 0));
    delay(500);
  }

  ctrl.end();
  
  PASS;
}

// Tests controlling a turnout
void testSetTurnout() {
  TEST;

  TrackController ctrl;

  ctrl.init(0, DEBUG, false);
  ctrl.begin();
  ctrl.setLocoSpeed(LOCO, 0);
  ctrl.setPower(true);

  ASSERT(0, ctrl.setTurnout(TURN, true));
  delay(500);
  ASSERT(1, ctrl.setTurnout(TURN, false));
  delay(500);

  ctrl.end();
  
  PASS;
}

// Tests reading/writing config values 
void testReadWriteConfig() {
  TEST;

  TrackController ctrl;

  ctrl.init(0, DEBUG, false);
  ctrl.begin();
  ctrl.setLocoSpeed(LOCO, 0);
  ctrl.setPower(true);

  byte saved = 3;
  byte value = 252;
  
  // Can only read for DCC, so assume true for other protocols
  ASSERT(0, LOCO < ADDR_DCC || ctrl.readConfig(LOCO, 2, &saved));
  
  delay(500);
  
  ASSERT(1, ctrl.writeConfig(LOCO, 2, 255 - saved));
  
  delay(500);

  // Can only read for DCC, so assume true for other protocols
  ASSERT(3, LOCO < ADDR_DCC || ctrl.readConfig(LOCO, 2, &value));
  ASSERT(4, value == 255 - saved);

  delay(500);

  ASSERT(5, ctrl.writeConfig(LOCO, 2, saved));
  
  delay(500);
  
  ctrl.end();
  
  PASS;
}
