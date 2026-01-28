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

// ---------------- WiFi Reconnect ----------------
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("WiFi lost, reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 5000) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi reconnected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi reconnect failed");
  }
}

// ---------------- MQTT Reconnect (non-blocking) ----------------
void ensureMQTT() {
  if (client.connected()) return;

  Serial.print("MQTT connecting... ");

  if (client.connect("ESP32_SHT40")) {
    Serial.println("connected");
    client.subscribe("test/topic");
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());
  }
}

// ---------------- SHT40 Sensor ----------------
#define I2C_SDA 4
#define I2C_SCL 3

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

// ---------------- Soil Moisture Sensor ----------------
#define SOIL_ADC_PIN 1      // ADC pin
#define SOIL_VCC_PIN 42     // GPIO powering the sensor

// Soil moisture timing
const unsigned long SOIL_ON_TIME  = 3UL * 60UL * 1000UL;   // 3 minutes
const unsigned long SOIL_OFF_TIME = 7UL * 60UL * 1000UL;   // 7 minutes
const unsigned long SOIL_READ_INTERVAL = 1000;             // 1 second

bool soilPowerOn = true;
unsigned long soilCycleStart = 0;
unsigned long lastSoilRead = 0;
int soilDiscard = 5;


// ---------------- Soil Read Function ----------------
int readSoilAveraged() {
  const int samples = 10;
  long sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(SOIL_ADC_PIN);
    delay(5);
  }
  return sum / samples;
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(200);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.print(" -> connected, IP: ");
  Serial.println(WiFi.localIP());

  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // SHT40
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT40");
    while (1) delay(1);
  }
  Serial.println("SHT40 online!");

  // Soil moisture power pin
  pinMode(SOIL_VCC_PIN, OUTPUT);
  digitalWrite(SOIL_VCC_PIN, HIGH);
  soilCycleStart = millis();

  Serial.println("Soil sensor ON for 3 minutes");
}


// ---------------- Loop ----------------
void loop() {
  // 1. WiFi recovery
  ensureWiFi();
  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    return;
  }

  // 2. MQTT recovery
  ensureMQTT();
  client.loop();

  unsigned long now = millis();

// ---------------- SHT40 every 2 seconds ----------------// 3. Sensor + publish every 2 seconds
  static unsigned long lastSHT = 0;

  if (millis() - lastSHT > 2000) {
    lastSHT = millis();

    sensors_event_t humidity, tempEvent;
    sht4.getEvent(&humidity, &tempEvent);

    float hum = humidity.relative_humidity;
    float temp = tempEvent.temperature;

    Serial.printf("Humidity: %.2f | Temperature: %.2f\n", hum, temp);

    char msg[16];

    snprintf(msg, sizeof(msg), "%.2f", hum);
    client.publish("sensors/humidity", msg);

    snprintf(msg, sizeof(msg), "%.2f", temp);
    client.publish("sensors/temperature", msg);
  }

  // ---------------- Soil Moisture Cycle ----------------
  unsigned long cycleElapsed = now - soilCycleStart;

  if (soilPowerOn) {
    // ON PHASE (3 minutes)
    if (cycleElapsed >= SOIL_ON_TIME) {
      soilPowerOn = false;
      soilCycleStart = now;
      digitalWrite(SOIL_VCC_PIN, LOW);
      Serial.println("Soil sensor OFF for 7 minutes");
    } else {
      // Read every 1 second
      if (now - lastSoilRead >= SOIL_READ_INTERVAL) {
        lastSoilRead = now;
        
        int soil = readSoilAveraged();
        
        if (soilDiscard > 0) {
          soilDiscard--;
          Serial.println("Discarding soil reading...");
        } else { 
          Serial.print("Soil raw value: ");
          Serial.println(soil); char msg[16];
          snprintf(msg, sizeof(msg), "%d", soil);
          client.publish("sensors/soil", msg);
        }
      }
    }

  } else {
    // OFF PHASE (7 minutes)
    if (cycleElapsed >= SOIL_OFF_TIME) {
      soilPowerOn = true;
      soilCycleStart = now;
      digitalWrite(SOIL_VCC_PIN, HIGH);
      Serial.println("Soil sensor ON for 3 minutes");
      soilDiscard = 5;
    }
  }
}

