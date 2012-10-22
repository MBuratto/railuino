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
const word    TIME  = 2000;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

TrackReporter rprt(2);

word track = 1;

void setup() {
  Serial.begin(115200);

  ctrl.begin();
  ctrl.setPower(true);
  ctrl.setLocoFunction(LOCO, 0, 1); 
  ctrl.setLocoSpeed(LOCO, 0); 
}

void waitForContact(word index) {
  Serial.print("Waiting for contact ");
  Serial.println(index);
  
  rprt.refresh();
  while (!rprt.getValue(index)) {
    rprt.refresh();
  }
  
  Serial.println("Ok");
}

void drivePastContact(int dir, int contact) {
  Serial.println("V200 go");
  ctrl.setLocoDirection(LOCO, dir);
  ctrl.setLocoSpeed(LOCO, SPEED);
  waitForContact(contact);

  Serial.println("V200 stop");
  ctrl.setLocoSpeed(LOCO, 0);
}

void loop() {
  drivePastContact(DIR_FORWARD, 1);

  delay(TIME);
  
  drivePastContact(DIR_REVERSE, 2);
  
  delay(TIME);
}

