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
#include <SoftwareSerial.h>

TrackController ctrl(0xdf24, true);

SoftwareSerial blue(4, 5);

String request;

String function;

word arguments[8];

word numOfArguments;

boolean result;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  ctrl.begin();
  blue.begin(9600);
  // Serial.println("100 Ready");
}

void receiveRequest() {
  char buffer[256];
  int length = 0;
  boolean newline = false;

  while (!newline && length < sizeof(buffer)) {
    if (blue.available() > 0) {
      int c = blue.read();
      
      if (c == 10) {
        buffer[length++] = 0;
        newline = true;
      } else if (c == 13) {
        // Ignore
      } else {
        buffer[length++] = c;
      }
    }
  }
  
  request = String(buffer);
  
  Serial.println(request);
}

word stringToWord(String s) {
  word result = 0;
  
  for (int i = 0; i < s.length(); i++) {
    result = 10 * result + (s.charAt(i) - '0');
  }
  
  return result;
}

boolean parse() {
  int lpar = request.indexOf('(');
  if (lpar == -1) {
    return false;
  }
  
  function = String(request.substring(0, lpar));
  function.trim();
  
  int offset = lpar + 1;
  int comma = request.indexOf(',', offset);
  numOfArguments = 0;
  while (comma != -1) {
    String tmp = request.substring(offset, comma);
    tmp.trim();
    arguments[numOfArguments++] = stringToWord(tmp);
    offset = comma + 1;
    comma = request.indexOf(',', offset);
  }

  int rpar = request.indexOf(')', offset);
  while (rpar == -1) {
    return false;
  }
  
  if (rpar > offset) {
    String tmp = request.substring(offset, rpar);
    tmp.trim();
    arguments[numOfArguments++] = stringToWord(tmp);
  }
  
  return true;
}

boolean dispatch() {
  if (function == "accelerateLoco") {
    return ctrl.accelerateLoco(arguments[0]);
  } else if (function == "decelerateLoco") {
    return ctrl.decelerateLoco(arguments[0]);
  } else if (function == "toggleLocoDirection") {
    return ctrl.toggleLocoDirection(arguments[0]);
  } else if (function == "setLocoDirection") {
    return ctrl.setLocoDirection(arguments[0], arguments[1]);
  } else if (function == "toggleLocoFunction") {
    return ctrl.toggleLocoFunction(arguments[0], arguments[1]);
  } else if (function == "setLocoFunction") {
    return ctrl.setLocoFunction(arguments[0], arguments[1], arguments[2]);
  } else if (function == "setTurnout") {
    return ctrl.setTurnout(arguments[0], arguments[1]);
  } else if (function == "setPower") {
    return ctrl.setPower(arguments[0]);
  } else if (function == "setLocoSpeed") {
    return ctrl.setLocoSpeed(arguments[0], arguments[1]);
  } else {
    return false;
  }
}

void loop() {
  receiveRequest();
  if (parse()) {
    if (dispatch()) {
      blue.println("200 Ok");
    } else {
      blue.println("300 Command error");
    }
  } else {
    blue.println("301 Syntax error");
  }
}

