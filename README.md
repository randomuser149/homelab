# ESP projects
So this is a microcontroller based sidequest so that I can monitor many things when I am not at home.
As of now I can monitor:
- humidity
- temperature
- soil moisture


## What you need for all of them:
- an ESP32 with WiFi (I have an ESP32S3 with WiFi)
- Arduino IDE installed
- an MQTT server you can sync to

# Configuration
The configurations for each project can be found below, separated under different titles.
## ESP32S3 SHT40 Humidity & Temperature
### What you need for this specific project
- an SHT40 sensor (and extra wires depending on your model)
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
>This code publishes to the MQTT topic `sensors/humidity` and `sensors/temperature` adjust it according to your needs.
```
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_SHT4x.h"

// ---------------- WiFi + MQTT ----------------
const char* ssid = "ASUS_58";
const char* password = "DaXeN2021";

const char* mqtt_server = "192.168.50.110";
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
```

Click upload and you should be good to go.  
I have to press a button on my board for the new code to start running once it finishes uploading, this may vary.

### Expected output

Over serial:
```
WiFi connecting.... -> connected, IP: 192.168.1.42
TCP test to broker 192.168.1.10:1883 ... SUCCESS (port reachable)
SHT40 online!
Humidity: 47.32 | Temperature: 24.28
Humidity: 47.28 | Temperature: 24.29
```
And live data over MQTT.
## ESP32S3 Capacitive Soil Moisture sensor
>[!IMPORTANT]
> These sensors aren't measuring true water content or anything like that.  
> They measure moisture that can vary based on the soil, the temperature, the uptime, the unit you have, how stable of a power source it has, and some other factors.  
> My plan is to use them as devices that alert me which plant needs to be watered, and THEY NEED CALIBRATION.  
> You need to have data and make thresholds for yourself about which plant can tolerate what outputs in the soil it's in.

### What you need for this specific project
- a capacitive soil moisture sensor (the ones that are full black, without exposed copper)
### Pin layout
| ESP32 Pin | Sensor Pin | Description        |
|-----------|------------|--------------------|
| 3V3       | VCC        | Power              |
| GND       | GND        | Ground             |
| GPIO3     | AOUT       | Analog output      |

### Arduino IDE
In Arduino IDE, you have to download the following libraries in library manager:    
- PubSubClient by Nick O'Leary
   
After you connect your ESP32 for the first time, it'll give you the prompt to automatically install drivers. Accept that.  

This code takes the data (averages of 10 readings) from the sensor every 3 seconds and publishes it to the sensors/soil topic.  
>[!IMPORTANT]
>My pin layout isn't set in stone, you can change it.  
>Replace ssid and password with your WiFi SSID and password.  
>Replace MQTT server LAN IP with your servers local IP address.  
>This code publishes to the MQTT topic `sensors/soil` adjust it according to your needs.
```
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

```
Click upload and you should be good to go.  
I have to press a button on my board for the new code to start running once it finishes uploading, this may vary.

### Expected output

Over serial:
```
WiFi connecting.................................................... -> connected, IP: 192.168.50.221
TCP test to broker 192.168.50.110:1883 ... SUCCESS (port reachable)
Continuous soil moisture monitoring started...
MQTT connecting. connected
Soil raw value: 2393
Soil raw value: 2397
...
```
And live data over MQTT.

## Monitoring
If something isn't going according to plan, you can open up the Serial Monitor in Arduino IDE and see what may cause the issue.  
The codes have some outputs for failed WiFi connection, failed MQTT connection and failed connections to sensors.
