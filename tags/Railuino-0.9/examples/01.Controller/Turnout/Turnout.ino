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

const word    TURN  = ADDR_ACC_MM2 + 1;
const word    TIME  = 5000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
  Serial.println("Power on");
  ctrl.setPower(true);
}


void loop() {
  Serial.println("Set turnout straight");
  ctrl.setTurnout(TURN, true);
 
  delay(TIME);
 
  Serial.println("Set turnout round");
  ctrl.setTurnout(TURN, false);
 
  delay(TIME);
}

