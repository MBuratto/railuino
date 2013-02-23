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

const word TIME = 1000;

TrackReporterIOX rprt(1); // Replace IOX by S88 when using S88.

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16");
  Serial.println("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
}

void loop() {
  rprt.refresh();

  for (int i = 1; i <= 16;i++) {
    Serial.print(rprt.getValue(i) ? "XX " : "   ");
  }
  Serial.println();
  
  delay(TIME);
}

