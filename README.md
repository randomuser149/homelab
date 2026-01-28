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
## ESP32S3 SHT40 Humidity & Temperature and Soil Moisture monitoring
### What you need for this specific project
- an SHT40 sensor (and extra wires depending on your model)
- a capacitive soil moisture sensor (the ones that are full black, without exposed copper)
### Pin layout
>[!NOTE]
>ESP32 boards allow flexible I²C pin assignment. If your board uses different default pins, adjust I2C_SDA and I2C_SCL accordingly.  

| ESP32 Pin | SHT40 Pin  | Description        |
|-----------|------------|--------------------|
| 3V3       | VDD        | Power              |
| GND       | GND        | Ground             |
| GPIO3     | SCL        | I²C Clock          |
| GPIO4     | SDA        | I²C Data           |

| ESP32 Pin | Soil Moisture Sensor Pin | Description        |
|-----------|--------------------------|--------------------|
| GPIO42    | VCC                      | Power              |
| GND       | GND                      | Ground             |
| GPIO1     | AOUT                     | Analog output      |

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
  
>[!IMPORTANT]
> Soil moisture sensors aren't measuring true water content or anything like that.  
> They measure moisture that can vary based on the soil, the temperature, the uptime, the unit you have, how stable of a power source it has, and some other factors.  
> My plan is to use them as devices that alert me which plant needs to be watered, and THEY NEED CALIBRATION.  
> You need to have data and make thresholds for yourself about which plant can tolerate what outputs in the soil it's in.  
  
This code publishes to the MQTT topic `sensors/humidity`, `sensors/temperature` and `sensors/soil`. Adjust it according to your needs.  
This code takes the data (averages of 10 readings) from the soil moisture sensor every seconds and publishes it for 3 minutes, then turns off for 7 minutes. It discards the first 5 readings for warmup misreads.  
It takes readings from the SHT40 sensor every 2 seconds continously.  
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

// ---------------- MQTT Reconnect ----------------
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

// ---------------- SHT40 every 2 seconds ----------------
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
```

Click upload and you should be good to go.  
I have to press a button on my board for the new code to start running once it finishes uploading, this may vary.

### Expected output (or similar to this)

Over serial:
```
WiFi connecting.... -> connected, IP: 192.168.1.10
MQTT connecting.... connected, IP: 192.168.1.10
SHT40 online!
Soil sensor ON for 3 minutes
Humidity: 47.32 | Temperature: 24.28
Soil raw value: 2393
Soil raw value: 2397
Humidity: 47.28 | Temperature: 24.29
Soil raw value: 2393
Soil raw value: 2397
```
And live data over MQTT.

## Monitoring
If something isn't going according to plan, you can open up the Serial Monitor in Arduino IDE and see what may cause the issue.  
The codes have some outputs for failed WiFi connection, failed MQTT connection and failed connections to sensors.