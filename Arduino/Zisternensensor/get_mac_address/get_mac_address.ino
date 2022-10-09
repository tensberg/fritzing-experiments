// see https://randomnerdtutorials.com/esp-now-esp8266-nodemcu-arduino-ide/

#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}

void loop() {

}
