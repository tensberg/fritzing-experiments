#include "secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>

const char* mqtt_server = "kochcloud.local";
const char* mqtt_username = "zisterne";
const char* mqtt_client = "temperatursensor_tobias";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const String TEMPERATURSENSOR_PATH = "zimmer_tobias/temperatursensor/";
const String ALERT_TOPIC = TEMPERATURSENSOR_PATH + "alert";
const String ALERT_SET_TOPIC = ALERT_TOPIC + "/set";
const String MESSAGE_TOPIC = TEMPERATURSENSOR_PATH + "message";
const String  MESSAGE_SHOWING_TOPIC = MESSAGE_TOPIC + "/showing";
const String  MESSAGE_ACKNOWLEDGED_TOPIC = MESSAGE_TOPIC + "/acknowledged";
const String  MESSAGE_SET_TOPIC = MESSAGE_TOPIC + "/set";

#define DHTPIN 2
#define DHTTYPE    DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3c
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SWITCH_PIN 12
#define LED_PIN 14
#define BUZZER_PIN 15

#define LOOP_DELAY_MILLIS 100
#define MEASUREMENT_PAUSE_SECONDS 60
#define DISPLAY_ON_SECONDS 5
#define DISPLAY_LOOP_COUNT DISPLAY_ON_SECONDS * 1000 / LOOP_DELAY_MILLIS;
#define ALERT_TOGGLE_SECONDS 1
#define ALERT_OFF_SECONDS 60

typedef struct {
  double temperature;
  double humidity;
} TemperatursensorData;

TemperatursensorData tsData;

#define MESSAGE_MAX_LENGTH 255

typedef struct {
  boolean showing;
  boolean acknowledged;
  char message[MESSAGE_MAX_LENGTH + 1];
} MessageData;

MessageData messageData;

int loopCount = 0;
int displayLoopCount = DISPLAY_LOOP_COUNT;
boolean switchPressed = false;
boolean alertIsOn = false;
boolean alertToggleHigh = false;
int alertLoopCount = 0;
boolean messageShowingChanged = false;
boolean messageAcknowledgedChanged = false;
boolean messageChanged = false;

void setup() {
  Serial.begin(115200);
  logInfo("Temperatursensor setup");

  initDisplay();
  initIos();
  initWifiClient();
  initMqtt();
  connectMqtt();
  dht.begin();

  messageData.message[0] = '\0';
  publishSwitchData();
  publishAlertData();
  publishMessageData();
}

void loop() {
  mqttClient.loop();
  updateAlert();
  mqttClient.loop();
  readSwitch();
  mqttClient.loop();
  updateMessage();
  if (loopCount++ % (MEASUREMENT_PAUSE_SECONDS*1000 / LOOP_DELAY_MILLIS) == 0) {
    measureData();
    displayData();
    publishData();
    mqttClient.loop();
  }
  updateDisplayState();
  mqttClient.loop();
  delay(LOOP_DELAY_MILLIS);
}

void logInfo(String message) {
  Serial.print("[INFO] ");
  Serial.println(message);
}

void initWifiClient() {
  WiFi.begin(wifi_client_ssid, wifi_client_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  logInfo("WiFi connected");
  logInfo("IP address: ");
  Serial.println(WiFi.localIP());
}

void initDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
  }
  
  logInfo("enabling display");
  display.display();
  delay(500);
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
}


void initMqtt() {
  mqttClient.setServer(mqtt_server, 1883);
}

void connectMqtt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    logInfo("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      mqttClient.unsubscribe("#");
      mqttClient.setCallback(mqttMessageReceived);
      subscribeMqtt(ALERT_SET_TOPIC);
      subscribeMqtt(MESSAGE_SHOWING_TOPIC);
      subscribeMqtt(MESSAGE_ACKNOWLEDGED_TOPIC);
      subscribeMqtt(MESSAGE_SET_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void subscribeMqtt(String topic) {
  boolean subscribed = mqttClient.subscribe(topic.c_str());
  mqttClient.loop();

  if (!subscribed) {
    Serial.print("failed to subscribe to ");
    Serial.println(topic);
  }
}

void mqttMessageReceived(char* topic, byte* payload, unsigned int len) {
  Serial.print("received MQTT message: ");
  Serial.println(topic);
  if (strcmp(topic, ALERT_SET_TOPIC.c_str()) == 0) {
    if (openhabStateToBool(payload, len)) {
      alertOn();
    } else {
      alertOff();
    }
  } else if (strcmp(topic, MESSAGE_SHOWING_TOPIC.c_str()) == 0) {
    boolean newShowing = openhabStateToBool(payload, len);
    if (newShowing != messageData.showing) {
      messageShowingChanged = true;
      messageData.showing = newShowing;
      if (messageData.showing && messageData.acknowledged) {
        messageData.acknowledged = false;
        messageAcknowledgedChanged = true;
      }
    }
  } else if (strcmp(topic, MESSAGE_ACKNOWLEDGED_TOPIC.c_str()) == 0) {
    boolean newAcknowledged = openhabStateToBool(payload, len);
    if (newAcknowledged != messageData.acknowledged) {
      messageData.acknowledged = newAcknowledged;
      messageAcknowledgedChanged = true;
    }
  } else if (strcmp(topic, MESSAGE_SET_TOPIC.c_str()) == 0) {
    int strEnd = min((int) len, MESSAGE_MAX_LENGTH);
    strncpy(messageData.message, (const char*) payload, strEnd);
    messageData.message[strEnd] = '\0';
    messageChanged = true;
  } else {
    Serial.println("unexpected MQTT topic");
  }

  updateMessage();
}

boolean openhabStateToBool(byte* payload, unsigned int len) {
  const char* str = (const char*) payload;
  if (strncmp("ON", str, len) == 0) {
    return true;
  } else if (strncmp("OFF", str, len) == 0) {
    return false;
  } else {
    Serial.println("unexpected state payload");
    return false;
  }
}

void measureData() {
  logInfo("measuring data");
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  tsData.temperature = event.temperature;
  dht.humidity().getEvent(&event);
  tsData.humidity = event.relative_humidity;
}

void displayData() {
  Serial.print("Temperature = ");
  Serial.print(tsData.temperature, 1);
  Serial.println(" *C");
  Serial.print("Humidity = ");
  Serial.print(tsData.humidity, 0);
  Serial.println(" %");
  if (messageData.showing) {
    Serial.print("message: ");
    Serial.println(messageData.message);
  }

  display.clearDisplay();
  display.setCursor(0, 0);

  if (messageData.showing) {
    display.println("================");
    display.println(messageData.message);
    display.println("================");
  }

  display.print("Temperature = ");
  display.print(tsData.temperature, 1);
  display.println(" *C");
  display.print("Humidity = ");
  display.print(tsData.humidity, 0);
  display.println(" %");
  
  display.display();
}

void publishData() {
  publishTemperatursensorData();
}

void publishTemperatursensorData() {
  publishMessage(TEMPERATURSENSOR_PATH + "sensor", "{\"temperature\":%.1f,\"humidity\":%.1f}", tsData.temperature, tsData.humidity);
}

void publishSwitchData() {
  publishMessage(TEMPERATURSENSOR_PATH + "switches", "{\"switch1\":\"%s\"}", boolToOpenhabState(switchPressed));
}

void publishAlertData() {
  publishMessage(ALERT_TOPIC, "{\"alert\":\"%s\"}", boolToOpenhabState(alertIsOn));
}

void publishMessageData() {
  publishMessageData(true, true, true);
}

void publishMessageData(boolean includeShowing, boolean includeAcknowledged, boolean includeMessage) {
  StaticJsonDocument<256> doc;
  if (includeShowing) {
    doc["showing"] = boolToOpenhabState(messageData.showing);
  }
  if (includeAcknowledged) {
    doc["acknowledged"] = boolToOpenhabState(messageData.acknowledged);
  }
  if (includeMessage) {
    doc["message"] = messageData.message;
  }

  char json[512];
  serializeJson(doc, json);
  publishMessage(MESSAGE_TOPIC, json);
}

String boolToOpenhabState(boolean b) {
  return b ? "ON" : "OFF";
}

void publishMessage(String path, const char* message, ...) {
  va_list params;
  va_start(params, message);

  char json[512];
  vsnprintf(json, sizeof(json), message, params);
  boolean success = mqttClient.publish(path.c_str(), json);
  Serial.print("[MQTT] ");
  Serial.print(path);
  Serial.print(" ");
  Serial.println(json);
  if (!success) {
    Serial.println("publish failed");
  }
}

void initIos() {
  pinMode(SWITCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  switchPressed = isSwitchPressed();
  ledOff();
  buzzerOff();
}

boolean isSwitchPressed() {
  return digitalRead(SWITCH_PIN) == LOW;
}

void readSwitch() {
  if (isSwitchPressed()) {
    if (!switchPressed) {
      handleSwitchPressed();
    }
  } else {
    if (switchPressed) {
      handleSwitchUnpressed();
    }
  }
}

void handleSwitchPressed() {
    switchPressed = true;
    if (alertIsOn) {
      alertOff();
    }
    if (messageData.showing && !messageData.acknowledged) {
      messageData.acknowledged = true;
      messageAcknowledgedChanged = true;
    }
    displayOn();
    loopCount = 0;
    publishSwitchData();
}

void handleSwitchUnpressed() {
  switchPressed = false;
  publishSwitchData();
}

void displayOn() {
  displayLoopCount = DISPLAY_LOOP_COUNT;
  logInfo("switching display on");
  display.ssd1306_command(SSD1306_DISPLAYON);
}

void displayOff() {
    logInfo("switching display off");
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void updateDisplayState() {
  if (displayLoopCount > 0 && (!messageData.showing || messageData.acknowledged)) {
    displayLoopCount--;
    if (displayLoopCount == 0) {
      displayOff();
      hideMessage();
    }
  }
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void buzzerOn() {
  tone(BUZZER_PIN, 440);
}

void buzzerOff() {
  noTone(BUZZER_PIN);
}

void alertOn() {
  alertIsOn = true;
  alertLoopCount = 0;
  publishAlertData();
  // alert starts in next loop
}

void alertOff() {
  ledOff();
  buzzerOff();
  alertIsOn = false;
  publishAlertData();
}

void updateAlert() {
  if (alertIsOn) {
    if (alertLoopCount % (ALERT_TOGGLE_SECONDS * 1000 / LOOP_DELAY_MILLIS) == 0) {
      if (alertToggleHigh) {
        ledOff();
        buzzerOff();
        alertToggleHigh = false;
      } else {
        ledOn();
        buzzerOn();
        alertToggleHigh = true;
      }
    }
  }
  if (alertLoopCount++ == ALERT_OFF_SECONDS * 1000 / LOOP_DELAY_MILLIS) {
    alertOff();
  }
}

void updateMessage() {
  if (messageShowingChanged || messageAcknowledgedChanged || messageChanged) {
    publishMessageData(messageShowingChanged, messageAcknowledgedChanged, messageChanged);
  }
  
  if (messageShowingChanged) {
    messageShowingChanged = false;
    if (messageData.showing) {
      displayOn();
      displayData();
    }
  }
  
  if (messageAcknowledgedChanged) {
    messageAcknowledgedChanged = false;
    if (messageData.acknowledged) {
      if (alertIsOn) {
        alertOff();
      }
    }
  }

  if (messageChanged) {
    messageChanged = false;
    if (messageData.showing) {
      displayData();
    }
  }
}

void hideMessage() {
  if (messageData.showing) {
    messageData.showing = false;
    publishMessageData(true, false, false);
  }
}
