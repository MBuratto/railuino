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
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
  Serial.println("Power on");
  ctrl.setPower(true);
}

void showRegister(int i, String label) {
  byte b;
  
  if (ctrl.readConfig(LOCO, i, &b)) {
    Serial.print("Register ");
    Serial.print(i, DEC);
    Serial.print(" - ");
    Serial.print(label);
    Serial.print(": ");
    Serial.println(b, DEC);
  }
}


void loop() {
  showRegister(1, "Address");
  showRegister(2, "Min. Voltage");
  showRegister(3, "Accel. time");
  showRegister(4, "Decel. time");
  showRegister(5, "Max. speed");
  showRegister(6, "Avg. speed");
  showRegister(7, "Version");
  showRegister(8, "Manufacturer");
  
  for (;;);
}

