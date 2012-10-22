/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */
 
#include <Railuino.h>

const word    LOCO  = ADDR_DCC + 21;
const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  ctrl.begin();
  Serial.println("Power on");
  ctrl.setPower(true);
  Serial.println("Headlights on");
  ctrl.setLocoFunction(LOCO, 0, 1);
}

void loop() {
  byte b;
  
  Serial.println("Direction forward");
  ctrl.setLocoDirection(LOCO, DIR_FORWARD);
  if (ctrl.getLocoDirection(LOCO, &b)) {
    Serial.print("(Direction is ");
    Serial.println(b == DIR_FORWARD ? "forward)" : "reverse)");
  }
  
  delay(TIME);
  
  Serial.println("Direction reverse");
  ctrl.setLocoDirection(LOCO, DIR_REVERSE);
  if (ctrl.getLocoDirection(LOCO, &b)) {
    Serial.print("(Direction is ");
    Serial.println(b == DIR_FORWARD ? "forward)" : "reverse)");
  }
  
  delay(TIME);
}

