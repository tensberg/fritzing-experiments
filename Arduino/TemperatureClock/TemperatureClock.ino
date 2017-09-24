/*
 * Show Time / Temperature / Humidity on a display
 *
 * Uses
 * - 4 segment LED controlled through 2 shift registers
 * - DHT Temperature / Humidity Sensor
 * - DS1307 Real-time clock
 */

#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

#define DHTPIN 8     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

const int DATA_PIN = 2;
const int CLOCK_PIN = 3;
const int LATCH_PIN = 4;

const int DIGITS[] = {
  2 + 4 + 8 + 16 + 32 + 64,
  4 + 8,
  2 + 4 + 16 + 32 + 128,
  2 + 4 + 8 + 16 + 128,
  4 + 8 + 64 + 128,
  2 + 8 + 16 + 64 + 128,
  2 + 8 + 16 + 32 + 64 + 128,
  2 + 4 + 8,
  2 + 4 + 8 + 16 + 32 + 64 + 128,
  2 + 4 + 8 + 16 + 64 + 128
};

#define MODE_TEMPERATURE 0
#define MODE_HUMIDITY 1
#define MODE_CLOCK 2

DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

int loopCount = 0;
DateTime now;
float humidity;
float temperature;
int mode = MODE_CLOCK;

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  clearSegments();

  if (! rtc.begin()) {
    while (1);
  }

  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  now = rtc.now();

  dht.begin();
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

void loop() {
  if (loopCount++ == 7500) {
    clearSegments();
    switch (mode) {
      case MODE_TEMPERATURE:
        mode = MODE_HUMIDITY;
        humidity = dht.readHumidity();
        break;
      case MODE_HUMIDITY:
        mode = MODE_CLOCK;
        now = rtc.now();
        break;
      case MODE_CLOCK:
        mode = MODE_TEMPERATURE;
        temperature = dht.readTemperature();
        break;
    }
    loopCount = 0;
  }

  switch (mode) {
    case MODE_TEMPERATURE:
      writeSegments(0, DIGITS[((int) temperature)/10] + 1);
      writeSegments(1, DIGITS[((int) temperature)%10]);
      writeSegments(2, DIGITS[((int) (temperature*10)) % 10]);
      writeSegments(3, DIGITS[((int) (temperature*100)) % 10]);
      break;
    case MODE_HUMIDITY:
      writeSegments(0, 0);
      writeSegments(1, 0);
      writeSegments(2, DIGITS[((int) humidity)/10]);
      writeSegments(3, DIGITS[((int) humidity)%10]);
      break;
    case MODE_CLOCK:
      writeSegments(0, DIGITS[now.hour()/10]);
      writeSegments(1, DIGITS[now.hour()%10] + (millis() / 1000) %2);
      writeSegments(2, DIGITS[now.minute()/10]);
      writeSegments(3, DIGITS[now.minute()%10]);
      break;
  }
}

void clearSegments() {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~0);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~0);
  digitalWrite(LATCH_PIN, HIGH);
}

void writeSegments(int digit, int segments) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~(1<<digit));
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~segments);
  digitalWrite(LATCH_PIN, HIGH);     
}

