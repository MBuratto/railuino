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
const word    TURN  = ADDR_ACC_MM2 + 1;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

const int UP    = A1;
const int DOWN  = A3;
const int LEFT  = A5;
const int RIGHT = A2;
const int FIRE  = A4;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
  
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
  
  ctrl.setPower(true);
  ctrl.setLocoDirection(LOCO, DIR_FORWARD);
  ctrl.setLocoFunction(LOCO, 0, 1);
}

int getJoystick() {
  if (digitalRead(UP) == 0) {
    return UP;
  } else if (digitalRead(RIGHT) == 0) {
    return RIGHT;
  } else if (digitalRead(DOWN) == 0) {
    return DOWN;
  } else if (digitalRead(FIRE) == 0) {
    return FIRE;
  } else if (digitalRead(LEFT) == 0) {
    return LEFT;
  } else {
    return 0;
  } 
}

int getJoystickWait() {
  int result = 0;
  
  while (result == 0) {
      result = getJoystick();
  }
  
  while (getJoystick() != 0) {
  };

  return result;
}

void loop() {
  int event = getJoystickWait();
  
  switch (event) {
    case UP: {
      ctrl.setTurnout(TURN, false);
      break;
    }
    case DOWN: {
      ctrl.setTurnout(TURN, true);
      break;
    }
    case RIGHT: {
      ctrl.accelerateLoco(LOCO);
      break;
    }
    case LEFT: {
      ctrl.decelerateLoco(LOCO);
      break;
    }
    case FIRE: {
      ctrl.setLocoDirection(LOCO, DIR_CHANGE);
      break;
    }
  }
}

