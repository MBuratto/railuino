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

const word PING = 1;
const word PONG = 2;

TrackReporterIOX rprt(1); // Replace IOX by S88 when using S88.

void setup() {
  Serial.begin(115200);
  while (!Serial);
}

void waitForContact(int i) {
  rprt.refresh();
  
  while (!rprt.getValue(i)) {
    rprt.refresh();
  }

  while (rprt.getValue(i)) {
    rprt.refresh();
  }
}

void loop() {
  waitForContact(PING);
  Serial.println("Ping");

  waitForContact(PONG);
  Serial.println("Pong");
}

