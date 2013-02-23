/*********************************************************************
 * Railuino - Hacking your Märklin
 *
 * Copyright (C) 2012 Joerg Pleumann
 * 
 * This example is free software; you can redistribute it and/or
 * modify it under the terms of the Creative Commons Zero License,
 * version 1.0, as published by the Creative Commons Organisation.
 * This effectively puts the file into the public domain.
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LICENSE file for more details.
 */
 
#include <Railuino.h>

const boolean DEBUG = false;

TrackController ctrl(0xdf24, DEBUG);

TrackMessage message;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println();
  Serial.println();
  Serial.println("Control locomotives using your MS2 to detect their address...");
  Serial.println();
  ctrl.begin();
}

void loop() {
  if (ctrl.receiveMessage(message)) {
    if (message.response && message.command >= 0x05 && message.command <= 0x07) {
      word full = makeWord(message.data[2], message.data[3]);
      
      String type = "unknown";
      word addr = 0;
      
      if (full >= ADDR_DCC) {
        type = "DCC";
        addr = full - ADDR_DCC;
      } else if (full >= ADDR_SX2) {
        type = "SX2";
        addr = full - ADDR_SX2;
      } else if (full >= ADDR_MFX) {
        type = "MFX";
        addr = full - ADDR_MFX;
      } else if (full >= ADDR_SX1) {
        type = "SX1";
        addr = full - ADDR_SX1;
      } else /* must be MM2 */ {
        type = "MM2";
        addr = full - ADDR_MM2;
      }
      
      Serial.println("Type is " + type + ", address is " + addr);
    }
  }
  
  delay(20);
}

