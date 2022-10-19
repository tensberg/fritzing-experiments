/*
 * Empfängt die vom Zisternensensor via ESPNow gesendeten Messdaten und veröffentlicht sie über den lokalen MQTT-Server.
 * 
 * Der ESP8266 kann nicht gleichzeitig auf einem WIFI-Kanal ESPNow-Nachrichten empfangen und auf einem anderen Kanal
 * also WIFI Client verbunden sein. (Jedenfalls habe ich es nicht hingekommen.) Deshalb wartet dieser Sketch als WIFI AP
 * auf eine ESPNow-Nachricht. Werden Daten empfangen, baut er eine WIFI Client Verbindung zum Hausnetz und darüber eine
 * Verbindung zum MQTT-Server auf. Nachdem er die MQTT-Nachricht gesendet hat, trennt er die WIFI Client Verbindung wieder.
 */

#include "secrets.h"
#include "src/common/configuration.h"
#include "src/common/secrets.h"

#include <espnow.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const String BRIDGE_PATH = "espnow_mqtt_bridge/";
const String ZISTERNENSENSOR_PATH = "garten/zisternensensor/";

WiFiClient wifiClient;

// MQTT

const char* mqtt_server = "kochcloud.local";
const char* mqtt_username = "zisterne";
const char* mqtt_client = "espnow_mqtt_bridge";

PubSubClient mqttClient(wifiClient);

ZisternensensorMessage zsMessage;
bool dataReceived = false;
int loopCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nsetup");

  initWifiAp();
  initEspNow();
  initMqtt();
}

void loop() {
  if (dataReceived) {
    publishData();
    Serial.println("data published");
    dataReceived = false;
  }

  delay(100);
}

// WIFI Modus zum Empfangen von ESPNow Nachrichten vom Zisternensensor
void initWifiAp() {
  WiFi.disconnect();
  wifi_set_channel(ap_wifi_channel);

  boolean result = WiFi.softAP(ap_ssid, ap_password, ap_wifi_channel, 0);
  Serial.println(result ? "WiFi AP ready" : "WiFi AP failed");
}

// WIFI Modus zur Verbindung ins Hausnetz zum MQTT-Server
void initWifiClient() {
  WiFi.disconnect();
  WiFi.begin(wifi_client_ssid, wifi_client_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initEspNow() {
  if (esp_now_init() != 0) {
    Serial.println("ESP Now initialization failed");
    delay(5000);
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  int res = esp_now_register_recv_cb(onDataRecv);
  if (res != 0) {
    Serial.print("failed to register recv cb: ");
    Serial.println(res);
  }
  Serial.println("ESPNow initialized");
}

void initMqtt() {
  mqttClient.setServer(mqtt_server, 1883);
}

void connectMqtt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

  Serial.println("Data received");
  if (len != sizeof(zsMessage)) {
    Serial.print("received unexpected data length: ");
    Serial.println(len);
    return;
  }

  memcpy(&zsMessage, incomingData, sizeof(zsMessage));
  dataReceived = true;
}

void publishData() {
  startPublish();
  publishBridgeStatus();
  publishZisternensensorData();
  publishZisternensensorStatus();
  endPublish();
}

void startPublish() {
  initWifiClient();
  connectMqtt();
}

void endPublish() {
  mqttClient.loop();
  initWifiAp();
}

void publishBridgeStatus() {
  publishMessage(BRIDGE_PATH + "status", "{\"rssi\":%d,\"channel\":%d}", WiFi.RSSI(), WiFi.channel());
}

void publishZisternensensorData() {
  publishMessage(ZISTERNENSENSOR_PATH + "sensor", "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.1f,\"distance\":%d}", zsMessage.data.temperature, zsMessage.data.humidity, zsMessage.data.pressure, zsMessage.data.distance);
}

void publishZisternensensorStatus() {
  publishMessage(ZISTERNENSENSOR_PATH + "status", "{\"iteration\":%lu,\"sleepTimeS\":%lu,\"activeTimeS\":%lu,\"sendCount\":{\"success\":%u,\"failed\":%u},\"sendStatus\":{\"last\":%d,\"lastFailed\":%d}}", 
      zsMessage.status.iteration, zsMessage.status.sleepTimeS, zsMessage.status.activeTimeS, 
      zsMessage.status.sendSuccessCount, zsMessage.status.sendFailedCount,
      zsMessage.status.lastSendStatus, zsMessage.status.lastSendFailedStatus);
}

void publishMessage(String path, const char* message, ...) {
  va_list params;
  va_start(params, message);

  char json[512];
  vsnprintf(json, sizeof(json), message, params);
  mqttClient.loop();
  delay(500); // somehow the MQTT lib needs a little think time between multiple publishes
  boolean success = mqttClient.publish(path.c_str(), json);
  Serial.print(path);
  Serial.print(" ");
  Serial.println(json);
  if (!success) {
    Serial.println("publish failed");
  }
}
