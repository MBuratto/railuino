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
  
  for (int i = 0; i <= 4; i++) {
    Serial.print("Function ");
    Serial.print(i, DEC);
    
    ctrl.setLocoFunction(LOCO, i, 1);
    if (ctrl.getLocoFunction(LOCO, i, &b)) {
      Serial.print("(Function ");
      Serial.print(i, DEC);
      Serial.println(b ? " is on)" : " is off)");
    }
  
    delay(TIME);

    ctrl.setLocoFunction(LOCO, i, 0);
    if (ctrl.getLocoFunction(LOCO, i, &b)) {
      Serial.print("(Function ");
      Serial.print(i, DEC);
      Serial.println(b ? " is on)" : " is off)");
    }
    
    delay(TIME);
  }
}

