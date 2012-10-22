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
}

void loop() {
  byte b;
  
  Serial.println("Lights on");
  ctrl.setLocoFunction(LOCO, 0, 1);
  if (ctrl.getLocoFunction(LOCO, 0, &b)) {
    Serial.print("(Lights are ");
    Serial.println(b ? "on)" : "off)");
  }
  
  delay(TIME);
  
  Serial.println("Lights off");
  ctrl.setLocoFunction(LOCO, 0, 0);
  if (ctrl.getLocoFunction(LOCO, 0, &b)) {
    Serial.print("(Lights are ");
    Serial.println(b ? "on)" : "off)");
  }
  
  delay(TIME);
}

