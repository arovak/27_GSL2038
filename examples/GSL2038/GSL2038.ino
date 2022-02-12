// driver for the GSL2038 touch panel
// Information gleaned from https://github.com/wireless-tag-com/8ms-esp32.git and various other sources
// firmware for the specific panel was found here:- https://github.com/wireless-tag-com/8ms-esp32
// As was some test code.
// Thanks to Skallwar for the source code this lib is bassed on : https://github.com/Skallwar/GSL1680


/*
Pin outs for Wireless Tag WT-86-32-3ZW1

gpio | function
---------------
14   | SCL
15   | SDA
12   | RST
13   | IRQ
*/

#include "GSL2038.h"

// Pins
#define TOUCH_SCL 14
#define TOUCH_SDA 15
#define TOUCH_RST 12
#define TOUCH_IRQ 13

GSL2038 TS = GSL2038();
// An other constructor is available to enable some debug on serial : GSL2038(bool error, bool info);

void setup () {
  Serial.begin(115200);
  delay(1000);
  if (Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)400000U))
  {
    Serial.println("Wire started");
  }
  else
  {
    Serial.println("Wire failed");
  }

  TS.begin(TOUCH_RST, TOUCH_IRQ);                 // Startup sequence CONTROLER part
}

void loop () {
  if(digitalRead(TOUCH_IRQ) == HIGH) {
    Serial.println("Touch");
    int NBFinger = TS.dataread();
    for(int i=0; i<NBFinger; i++){
      Serial.print(TS.readFingerID(i));
      Serial.print(" ");
      Serial.print(TS.readFingerX(i));
      Serial.print(" ");
      Serial.print(TS.readFingerY(i));
      Serial.println("");
    }
    Serial.println("---");
  }
}
