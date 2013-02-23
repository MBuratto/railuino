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

const word    LOCO  = ADDR_MM2 + 78;
const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
  Serial.println("Power on");
  ctrl.setPower(true);
  Serial.println("Headlights on");
  ctrl.setLocoFunction(LOCO, 0, 1);
}

void loop() {
  byte b;
  
  Serial.println("Direction forward");
  ctrl.setLocoDirection(LOCO, DIR_FORWARD);
  if (ctrl.getLocoDirection(LOCO, &b)) {
    Serial.print("(Direction is ");
    Serial.println(b == DIR_FORWARD ? "forward)" : "reverse)");
  }
  
  delay(TIME);
  
  Serial.println("Direction reverse");
  ctrl.setLocoDirection(LOCO, DIR_REVERSE);
  if (ctrl.getLocoDirection(LOCO, &b)) {
    Serial.print("(Direction is ");
    Serial.println(b == DIR_FORWARD ? "forward)" : "reverse)");
  }
  
  delay(TIME);
}

