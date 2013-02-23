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

word ADDRESS[] = { DELTA1, DELTA2, DELTA3, DELTA4 };

word loco = ADDRESS[0];

TrackController ctrl(0xdf24, true);

int input(String message, int low, int high) {
  while(true) {
    int result = 0;

    Serial.println();
    Serial.print(message);
    Serial.print(" (");
    Serial.print(low, DEC);
    Serial.print(" - ");
    Serial.print(high, DEC);
    Serial.println(")");
  
    while (true) {
      if (Serial.available() > 0) {
        int c = Serial.read();
        
        if ((c == 10 || c == 13) && result > 0) {
          if (result >= low && result <= high) {
            Serial.println();
            Serial.print(">> ");
            Serial.println(result, DEC);
            return result;
          } else {
            break;
          }
        } else if (c >= '0' && c <= '9') {
          result = result * 10 + (c - '0');
        } else {
          result = 0;
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();

  for (int i = 0; i < 20; i++) {
    Serial.println();
  }
  
  Serial.println("Railuino serial console");
}

void loop() {
  Serial.println();
  Serial.println("1 ... Choose loco");
  Serial.println("2 ... Accelerate loco");
  Serial.println("3 ... Decelerate loco");
  Serial.println("4 ... Toggle direction");
  Serial.println("5 ... Toggle headlights");
  Serial.println("6 ... Switch turnout");
  Serial.println("7 ... Set power on");
  Serial.println("8 ... Set power off");
  
  int cmd = input("Choose command", 1, 8);
  
  if (cmd == 1) {
    loco = ADDRESS[input("Choose loco", 1, 4) - 1];
  } else if (cmd == 2) {
    ctrl.accelerateLoco(loco);
  } else if (cmd == 3) {
    ctrl.decelerateLoco(loco);
  } else if (cmd == 4) {
    ctrl.toggleLocoDirection(loco);
  } else if (cmd == 5) {
    ctrl.toggleLocoFunction(loco, 0);
  } else if (cmd == 6) {
    int turnout = input("Choose turnout", 1, 16);
    int value = input("Choose direction [1=straight, 2=round]", 1, 2);
    ctrl.setTurnout(ADDR_ACC_MM2 + turnout, value == 1);
  } else if (cmd == 7) {
    ctrl.setPower(true);
  } else if (cmd == 8) {
    ctrl.setPower(false);
  }
}

