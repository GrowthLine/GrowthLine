#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_ILI9341.h>

Adafruit_STMPE610 touch = Adafruit_STMPE610(8);

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit STMPE610 example");
  Serial.flush();

  pinMode(10, OUTPUT);
  if (! touch.begin()) {
    Serial.println("STMPE not found!");
    while(1);
  }
  Serial.println("Waiting for touch sense");
}

void loop() {
  uint16_t x, y;
  uint8_t z;
  if (touch.touched()) {
    // read x & y & z;
    while (! touch.bufferEmpty()) {
      Serial.print(touch.bufferSize());
      touch.readData(&x, &y, &z);
      Serial.print("->("); 
      Serial.print(x); Serial.print(", "); 
      Serial.print(y); Serial.print(", "); 
      Serial.print(z);
      Serial.println(")");
    }
    touch.writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
  }
  delay(10);

}
