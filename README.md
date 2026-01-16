# ESP 32
So this is a microcontroller based sidequest so that I can monitor my room temperature and humidity from anywhere.  
So far, the humidity is implemented and syncs to the database I created which is present in the cloudflared branch.

## What you need
- an ESP32 (I have an ESP32S3 with WiFi)
- an SHT40 sensor (and extra wires depending on your model)
- Arduino IDE installed
- an MQTT server you can sync to

## Configuration

### Pin layout
>[!NOTE]
>ESP32 boards allow flexible I²C pin assignment. If your board uses different default pins, adjust I2C_SDA and I2C_SCL accordingly.  

| ESP32 Pin | Sensor Pin | Description        |
|-----------|------------|--------------------|
| 3V3       | VDD        | Power              |
| GND       | GND        | Ground             |
| GPIO3     | SCL        | I²C Clock          |
| GPIO4     | SDA        | I²C Data           |

### Arduino IDE
In Arduino IDE, you have to download the following libraries in library manager:  
- Adafruit SHT4x Library by Adafruit  
- PubSubClient by Nick O'Leary
   
After you connect your ESP32 for the first time, it'll give you the prompt to automatically install drivers. Accept that.
Then, paste in the following code.
>[!IMPORTANT]
>My pin layout isn't set in stone, you can change it.  
>Replace ssid and password with your WiFi SSID and password.  
>Replace MQTT server LAN IP with your servers local IP address.  
>This code publishes to the MQTT topic `sensors/humidity` adjust it according to your needs.
```
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

    sensors_event_t humidity, temp;
    sht4.getEvent(&humidity, &temp);

    float hum = humidity.relative_humidity;

    Serial.print("Humidity: ");
    Serial.println(hum);

    // Convert to string for MQTT
    char msg[16];
    snprintf(msg, sizeof(msg), "%.2f", hum);

    client.publish("sensors/humidity", msg);
  }
}
```

Click upload and you should be good to go.  
I have to press a button on my board for the new code to start running once it finishes uploading, this may vary.

### Monitoring
If something isn't going according to plan, you can open up the Serial Monitor in Arduino IDE and see what may cause the issue.  
This code has some outputs for failed WiFi connection, failed MQTT connection and failed connection to the SHT40 sensor.

## Expected output
Over serial:
```
WiFi connecting.... -> connected, IP: 192.168.1.42
TCP test to broker 192.168.1.10:1883 ... SUCCESS (port reachable)
SHT40 online!
Humidity: 47.32
Humidity: 47.28
```
And live data over MQTT.
