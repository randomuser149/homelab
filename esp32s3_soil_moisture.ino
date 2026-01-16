#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

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

    bool ok = client.connect("ESP32_SOIL_MOISTURE");
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

// ---------------- Soil Moisture Sensor ----------------
#define SOIL_ADC_PIN 3     // ADC1 pin

unsigned long lastRead = 0;

int readSoilAveraged() {
  const int samples = 10;
  long sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(SOIL_ADC_PIN);
    delay(5);
  }

  return sum / samples;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // WiFi
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

  Serial.println("Continuous soil moisture monitoring started...");
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // Read every 3 seconds
  if (now - lastRead > 3000) {
    lastRead = now;

    int soil_m = readSoilAveraged();

    Serial.print("Soil raw value: ");
    Serial.println(soil_m);

    char msg[16];
    snprintf(msg, sizeof(msg), "%d", soil_m);

    client.publish("sensors/soil", msg);
  }
}
