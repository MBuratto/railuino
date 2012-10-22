/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */
 
#include <Railuino.h>

const word    TURN  = ADDR_ACC_MM2 + 1;
const word    TIME  = 5000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
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

