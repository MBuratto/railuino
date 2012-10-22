/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */

#include <Railuino.h>

const word    LOCO  = ADDR_DCC + 21;
const word    SPEED = 100;
const word    TIME  = 5000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  
  Serial.println("Power on");
  ctrl.setPower(true);
  Serial.println("Headlights on");
  ctrl.setLocoFunction(LOCO, 0, 1); 
  Serial.println("Direction forward");
  ctrl.setLocoDirection(LOCO, DIR_FORWARD);
}

void loop() {
  word speed;
  
  Serial.print("Setting speed to ");
  Serial.println(SPEED);
  ctrl.setLocoSpeed(LOCO, SPEED);
  if (ctrl.getLocoSpeed(LOCO, &speed)) {
    Serial.print("(Speed is ");
    Serial.print(speed, DEC);
    Serial.println(")");
  }
  
  delay(TIME);
  
  Serial.println("Stop");
  ctrl.setLocoSpeed(LOCO, 0);
  if (ctrl.getLocoSpeed(LOCO, &speed)) {
    Serial.print("(Speed is ");
    Serial.print(speed, DEC);
    Serial.println(")");
  }

  Serial.println("System stopped. Need to reset.");
  
  for (;;);
}

