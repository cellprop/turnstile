#include <HardwareSerial.h>

void setup() {
  Serial.begin(9600); // Default RX and TX for ESP8266 (GPIO 3 and GPIO 1)
  Serial.println("Ready");
}

void loop() {
  if (Serial.available()) {
    String str = Serial.readString();
    Serial.print(str);
  }
  delay(1000);
}