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

const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

TrackMessage message;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println();
  Serial.println();
  Serial.println("DIR CMND R HASH HASH LNGT DAT0 DAT1 DAT2 DAT3 DAT4 DAT5 DAT6 DAT7");
  Serial.println("--- ---- - ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----");
  ctrl.begin();
}

void loop() {
  ctrl.receiveMessage(message);
  delay(20);
}

