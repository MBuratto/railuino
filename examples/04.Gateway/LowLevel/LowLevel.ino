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
#include <SoftwareSerial.h>

TrackController ctrl(0xdf24, false);

SoftwareSerial blue(10, 11);

TrackMessage message;

char buffer[32];

int length;

void setup() {
  ctrl.begin();
}

void loop() {
  if (ctrl.receiveMessage(message)) {
    blue.println(message);
  }
  
  if (blue.available()) {
    char c = blue.read();
    
    if (c == 10 || length == sizeof(buffer)) {
      String s = String(buffer);
      length = 0;
      if (message.parseFrom(s)) {
        ctrl.sendMessage(message);
      }
    } else if (c >= 32) {
      buffer[length++] = c;
    }
  }
  
}

