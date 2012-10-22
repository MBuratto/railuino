/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */
 
#include <Railuino.h>

const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  ctrl.begin();
}

void loop() {
  Serial.println("Power on");
  ctrl.setPower(true);
  
  delay(TIME);
  
  Serial.println("Power off");
  ctrl.setPower(false);
  
  delay(TIME);
}

