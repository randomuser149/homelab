#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_SHT4x.h"

// ---------------- WiFi + MQTT ----------------
const char* ssid = "ssid";
const char* password = "password";

const char* mqtt_server = "MQTT server LAN IP";
const int   mqtt_port   = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- SHT40 Sensor ----------------
#define I2C_SDA 4
#define I2C_SCL 3

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

// ---------------- MQTT Callback ----------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT msg [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ---------------- Debug TCP Test ----------------
void debugTcpTest() {
  Serial.print("TCP test to broker ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.print(mqtt_port);
  Serial.print(" ... ");

  WiFiClient testClient;
  if (testClient.connect(mqtt_server, mqtt_port)) {
    Serial.println("SUCCESS (port reachable)");
    testClient.stop();
  } else {
    Serial.println("FAILED (port unreachable)");
  }
}

// ---------------- MQTT Reconnect ----------------
void reconnect() {
  Serial.print("MQTT connecting");

  while (!client.connected()) {
    Serial.print(".");

    bool ok = client.connect("ESP32_SHT40");
    if (ok) {
      Serial.println(" connected");
      client.subscribe("test/topic");
      return;
    }

    Serial.print(" failed, rc=");
    Serial.print(client.state());
    Serial.println(" retrying in 1s");
    delay(1000);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(200);

  // WiFi
  WiFi.disconnect(true);
  delay(200);

  WiFi.mode(WIFI_STA);
  delay(200);

  Serial.print("WiFi connecting");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.print(" -> connected, IP: ");
  Serial.println(WiFi.localIP());

  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  delay(500);
  debugTcpTest();

  // SHT40
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT40");
    while (1) delay(1);
  }
  Serial.println("SHT40 online!");
}

// ---------------- Loop ----------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long last = 0;

  if (millis() - last > 2000) {  // every 2 seconds
    last = millis();

    sensors_event_t humidity, tempEvent;
    sht4.getEvent(&humidity, &tempEvent);

    float hum = humidity.relative_humidity;
    float temp = tempEvent.temperature;

    Serial.printf("Humidity: %.2f | Temperature: %.2f\n",hum,temp);

    // Convert to string for MQTT
    char msg[16];
    snprintf(msg, sizeof(msg), "%.2f", hum);

    client.publish("sensors/humidity", msg);

    snprintf(msg, sizeof(msg), "%.2f", temp);
    client.publish("sensors/temperature", msg);

  }
}
