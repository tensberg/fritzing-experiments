#include "src/common/configuration.h"
#include "src/common/secrets.h"

#include <ESP8266WiFi.h>
#include <espnow.h>

// Environment Sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Distance Sensor
#include <NewPing.h>

#define vcc3VPin 13
#define vcc5VPin 15

// environment sensor
#define BME_I2C_ADDRESS 0x76

Adafruit_BME280 bme; // I2C

// display sensor
// Define Trig and Echo pin:
#define trigPin 14
#define echoPin 12

// Define maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500 cm:
#define MAX_DISTANCE 400

NewPing sonar(trigPin, echoPin, MAX_DISTANCE);

sensordata data;
int message_counter = 0;

struct send_status {
  int sendSuccess;
  int sendFailed;
  int lastSendStatus;
} send_status;

#define ENABLE_DISPLAY

#ifdef ENABLE_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif

void setup() {
  Serial.begin(57600);
  Serial.println("\n\n\nHello Sensor");

  pinMode(vcc3VPin, OUTPUT);
  digitalWrite(vcc3VPin, HIGH);
  pinMode(vcc5VPin, OUTPUT);
  digitalWrite(vcc5VPin, HIGH);

  initEspNow();
  initBme();
  initDisplay();
}

int loop_counter = 0;

void loop() {
  if ((loop_counter++ % 100) == 0) {
    data.temperature = bme.readTemperature();
    data.pressure = bme.readPressure() / 100.0F;
    data.humidity = bme.readHumidity();
    data.distance = sonar.convert_cm(sonar.ping_median());
    //data.distance = message_counter++; // for testing only
  
    int res = esp_now_send(NULL, (uint8_t *) &data, sizeof(data));
    if (res != 0) {
      Serial.print("send failed: ");
      Serial.println(res);
    }

    displayData();
  }


  delay(50);
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

void dataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status);
  send_status.lastSendStatus = status;
  if (status == 0) {
    send_status.sendSuccess++;
  } else {
    send_status.sendFailed++;
  }
}


void displayData() {
  Serial.print("Temperature = ");
  Serial.print(data.temperature, 1);
  Serial.println(" *C");
  Serial.print("Pressure = ");
  Serial.print(data.pressure, 0);
  Serial.println(" hPa");
  Serial.print("Humidity = ");
  Serial.print(data.humidity, 0);
  Serial.println(" %");
  Serial.print("Distance = ");
  Serial.print(data.distance);
  Serial.println(" cm");
  
#ifdef ENABLE_DISPLAY
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Temperature = ");
  display.print(data.temperature, 1);
  display.println(" *C");
  display.print("Pressure = ");
  display.print(data.pressure, 0);
  display.println(" hPa");
  display.print("Humidity = ");
  display.print(data.humidity, 0);
  display.println(" %");
  display.print("Distance = ");
  display.print(data.distance);
  display.println(" cm");
  display.print("last send status:");
  display.println(send_status.lastSendStatus);
  display.print("send success: ");
  display.println(send_status.sendSuccess);
  display.print("send failed: ");
  display.println(send_status.sendFailed);

  //digitalWrite(vcc5VPin, send_status.sendFailed%2 == 0 ? HIGH : LOW);
  
  display.display();
#endif
}
