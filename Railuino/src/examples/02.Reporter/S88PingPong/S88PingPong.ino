/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */

#include <Railuino.h>

const word PING = 1;
const word PONG = 2;

TrackReporter rprt(16);

void setup() {
  Serial.begin(115200);
}

void waitForContact(int i) {
  rprt.refresh();
  
  while (!rprt.getValue(i)) {
    rprt.refresh();
  }

  while (rprt.getValue(i)) {
    rprt.refresh();
  }
}

void loop() {
  waitForContact(PING);
  Serial.println("Ping");

  waitForContact(PONG);
  Serial.println("Pong");
}

