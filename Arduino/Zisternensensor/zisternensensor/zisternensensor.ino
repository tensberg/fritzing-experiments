#include "src/common/configuration.h"
#include "src/common/secrets.h"

// Wifi
#include <ESP8266WiFi.h>
#include <espnow.h>

// Environment Sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Distance Sensor
#include <NewPing.h>

// RTCMemory
#include <RTCMemory.h>

// features for testing / debugging
//#define ENABLE_SERIAL_STATUS
//#define ENABLE_DISPLAY


// Pin definitions
#define VCC_3V_PIN 5
#define VCC_5V_PIN 15
#define DISTANCE_TRIG_PIN 14
#define DISTANCE_ECHO_PIN 12
#define I2C_SDA 0
#define I2C_SCL 4

// pause between measurements
#define MICROS 1000000
#define SLEEP_TIME_MICROS 120 * MICROS

// environment sensor
#define BME_I2C_ADDRESS 0x76
Adafruit_BME280 bme;

// distance sensor
// Define maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500 cm:
#define MAX_DISTANCE 400
NewPing sonar(DISTANCE_TRIG_PIN, DISTANCE_ECHO_PIN, MAX_DISTANCE);

#ifdef ENABLE_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3c
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif

RTCMemory<ZisternensensorStatus> rtcMemory("/etc/zisternensensor_status.bin");
ZisternensensorStatus *zsStatus;
ZisternensensorMessage zsMessage;
ZisternensensorData *zsData = &zsMessage.data;

unsigned long sketchStartMicros = micros();

void setup() {
  // init
  Serial.begin(57600);
  Serial.println("\n\n\nsetup");

  initZisternensensorStatus();

  pinMode(VCC_3V_PIN, OUTPUT);
  pinMode(VCC_5V_PIN, OUTPUT);
  sensorsOn();

  Wire.pins(I2C_SDA, I2C_SCL);
  initEspNow();
  initBme();
  initDisplay();

  measureData();
  sensorsOff();
  sendDataEspNow();
  displayData();

  // end iteration and sleep to next measurement
  updateAndSaveStatus();
  ESP.deepSleep(SLEEP_TIME_MICROS);
}

void loop() {
  // we should never get here because deepSleep is ended by a reset
  Serial.println("looping");
}

void initZisternensensorStatus() {
  Serial.print("Filesystem initialization... ");
  if (LittleFS.begin()) {
    Serial.println("Done!");
  } else {
    Serial.println("Error");
  }

  if (rtcMemory.begin()) {
    Serial.println("Initialization done! Previous data found.");
  } else {
    Serial.println("Initialization done! No previous data found. The buffer is cleared.");
  }

  zsStatus = rtcMemory.getData();

  
#ifdef ENABLE_SERIAL_STATUS
  Serial.print("iteration:");
  Serial.println(zsStatus->iteration);
  Serial.println("sleep time: ");
  prettyPrintTime(zsStatus->sleepTimeS);
  Serial.println();
  Serial.println("active time: ");
  prettyPrintTime(zsStatus->activeTimeS);
  Serial.print(".");
  Serial.println(zsStatus->activeTimeMicros);
  Serial.print("send success: ");
  Serial.println(zsStatus->sendSuccessCount);
  Serial.print("send failed: ");
  Serial.println(zsStatus->sendFailedCount);
  Serial.print("last send status:");
  Serial.println(zsStatus->lastSendStatus);
  Serial.print("last send failed status:");
  Serial.println(zsStatus->lastSendFailedStatus);
#endif
}

void prettyPrintTime(long seconds) {
  Serial.print(seconds / 60 / 60 / 24);
  Serial.print("D");
  Serial.print(seconds / 60 / 60 % 24);
  Serial.print(":");
  Serial.print(seconds / 60 % 60);
  Serial.print(":");
  Serial.print(seconds % 60);
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != 0) {
    Serial.println("ESP Now initialization failed");
    delay(5000);
    ESP.restart();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  int res = esp_now_add_peer(mqtt_bridge_mac_address, ESP_NOW_ROLE_CONTROLLER, ap_wifi_channel, NULL, 0);
  if (res==0) {
    Serial.println("Erfolgreich gepaart");
  } else {
    Serial.print("esp_now paarung fehlgeschlagen: ");
    Serial.println(res);
  }
  //register callback function
  res = esp_now_register_send_cb(dataSent);
  if (res != 0) {
    Serial.print("failed to register send cb: ");
    Serial.println(res);
  }
}

void initBme() {
  bool communication = bme.begin(BME_I2C_ADDRESS);
  if (!communication) {
    Serial.println("Could not find a valid BME280 sensor");
  }
  else {
    Serial.println("BME Environment Sensor Communication established!");
  }
}

void initDisplay() {
#ifdef ENABLE_DISPLAY

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  Serial.println(F("enabling display"));
  display.display();
  delay(500);
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
#endif
}

void measureData() {
  zsData->temperature = bme.readTemperature();
  zsData->pressure = bme.readPressure() / 100.0F;
  zsData->humidity = bme.readHumidity();
  zsData->distance = sonar.convert_cm(sonar.ping_median());
}

void sendDataEspNow() {
  // copy status from RTCMemory to zsMessage struct
  memcpy(&zsMessage.status, zsStatus, sizeof(*zsStatus));
  
  int res = esp_now_send(NULL, (uint8_t *) &zsMessage, sizeof(zsMessage));
  if (res != 0) {
    Serial.print("send failed: ");
    Serial.println(res);
  }
}

void dataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status);
  zsStatus->lastSendStatus = status;
  if (status == 0) {
    zsStatus->sendSuccessCount++;
  } else {
    zsStatus->sendFailedCount++;
    zsStatus->lastSendFailedStatus = status;
  }
}


void displayData() {
#ifdef ENABLE_SERIAL_STATUS
  Serial.print("Temperature = ");
  Serial.print(zsData->temperature, 1);
  Serial.println(" *C");
  Serial.print("Pressure = ");
  Serial.print(zsData->pressure, 0);
  Serial.println(" hPa");
  Serial.print("Humidity = ");
  Serial.print(zsData->humidity, 0);
  Serial.println(" %");
  Serial.print("Distance = ");
  Serial.print(zsData->distance);
  Serial.println(" cm");
#endif
  
#ifdef ENABLE_DISPLAY
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Temperature = ");
  display.print(zsData->temperature, 1);
  display.println(" *C");
  display.print("Pressure = ");
  display.print(zsData->pressure, 0);
  display.println(" hPa");
  display.print("Humidity = ");
  display.print(zsData->humidity, 0);
  display.println(" %");
  display.print("Distance = ");
  display.print(zsData->distance);
  display.println(" cm");
  display.print("last send status:");
  display.println(zsStatus->lastSendStatus);
  display.print("send success: ");
  display.println(zsStatus->sendSuccessCount);
  display.print("send failed: ");
  display.println(zsStatus->sendFailedCount);
  
  display.display();
#endif
}

void sensorsOn() {
  switchSensorsVcc(HIGH);
}

void sensorsOff() {
  switchSensorsVcc(LOW);
}

void switchSensorsVcc(int value) {
  digitalWrite(VCC_3V_PIN, value);
  digitalWrite(VCC_5V_PIN, value);
}

void updateAndSaveStatus() {
  zsStatus->iteration++;
  zsStatus->sleepTimeS += SLEEP_TIME_MICROS / MICROS;
  unsigned long activeTimeMicros = micros() - sketchStartMicros;
  zsStatus->activeTimeMicros += activeTimeMicros;
  zsStatus->activeTimeS += zsStatus->activeTimeMicros / MICROS;
  zsStatus->activeTimeMicros %= MICROS;

  rtcMemory.save();

  // don't backup on every iteration to preserve battery
  if (zsStatus->iteration%10 == 0) {
   rtcMemory.backup();
  }
}
