/*************************************************
 * Railuino - Hacking your Märklin
 *
 * (c) 2012 Joerg Pleumann
 *
 * TODO: Add proper license text here.
 */
 
#include <Railuino.h>

const word    LOCO  = ADDR_DCC + 21;
const boolean DEBUG = true;

TrackController ctrl(0xdf24, DEBUG);

void setup() {
  Serial.begin(115200);
  ctrl.begin();
  Serial.println("Power on");
  ctrl.setPower(true);
}

void showRegister(int i, String label) {
  byte b;
  
  if (ctrl.readConfig(LOCO, i, &b)) {
    Serial.print("Register ");
    Serial.print(i, DEC);
    Serial.print(" - ");
    Serial.print(label);
    Serial.print(": ");
    Serial.println(b, DEC);
  }
}


void loop() {
  showRegister(1, "Address");
  showRegister(2, "Min. Voltage");
  showRegister(3, "Accel. time");
  showRegister(4, "Decel. time");
  showRegister(5, "Max. speed");
  showRegister(6, "Avg. speed");
  showRegister(7, "Version");
  showRegister(8, "Manufacturer");
  
  for (;;);
}

