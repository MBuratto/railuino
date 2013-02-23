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
const word    SPEED = 100;
const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

TrackReporterIOX rprt(1); // Replace IOX by S88 when using S88.

word track = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  ctrl.begin();
  ctrl.setPower(true);
  ctrl.setLocoFunction(LOCO, 0, 1); 
  ctrl.setLocoSpeed(LOCO, 0); 
}

void waitForContact(word index) {
  Serial.print("Waiting for contact ");
  Serial.println(index);
  
  rprt.refresh();
  while (!rprt.getValue(index)) {
    rprt.refresh();
  }
  
  Serial.println("Ok");
}

void drivePastContact(int dir, int contact) {
  Serial.println("V200 go");
  ctrl.setLocoDirection(LOCO, dir);
  ctrl.setLocoSpeed(LOCO, SPEED);
  waitForContact(contact);

  Serial.println("V200 stop");
  ctrl.setLocoSpeed(LOCO, 0);
}

void loop() {
  drivePastContact(DIR_FORWARD, 1);

  delay(TIME);
  
  drivePastContact(DIR_REVERSE, 2);
  
  delay(TIME);
}

