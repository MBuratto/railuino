/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */
 
#include <Railuino.h>

const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

TrackMessage message;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("DIR CMND R HASH HASH LNGT DAT0 DAT1 DAT2 DAT3 DAT4 DAT5 DAT6 DAT7");
  Serial.println("--- ---- - ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----");
  ctrl.begin();
}

void loop() {
  ctrl.receiveMessage(&message);
  delay(20);
}

