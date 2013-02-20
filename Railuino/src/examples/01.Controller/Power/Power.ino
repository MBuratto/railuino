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

const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
}

void loop() {
  Serial.println("Power on");
  ctrl.setPower(true);
  
  delay(TIME);
  
  Serial.println("Power off");
  ctrl.setPower(false);
  
  delay(TIME);
}

