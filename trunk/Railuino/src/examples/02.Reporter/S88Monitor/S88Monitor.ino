/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */

#include <Railuino.h>

const word TIME = 1000;

TrackReporter rprt(16);

void setup() {
  Serial.begin(115200);
  Serial.println("01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16");
  Serial.println("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
}

void loop() {
  rprt.refresh();

  for (int i = 1; i <= 16;i++) {
    Serial.print(rprt.getValue(i) ? "XX " : "   ");
  }
  Serial.println();
  
  delay(TIME);
}

