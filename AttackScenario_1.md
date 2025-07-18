
---

## Attack Scenario 1: Anonymous Access to MQTT Broker

### **Goal:** Show that an attacker can connect to MQTT broker **without authentication**, publish arbitrary messages (e.g., LED on/off), and subscribe to sensitive topics (e.g., DHT data).

---

## Setup Before the Attack

### Assumptions:

* MQTT Broker is running on a **Raspberry Pi** using **Mosquitto**
* ESP32 is:

  * Publishing **DHT data** to `esp32/dht`
  * Subscribing to `esp32/led` for control

---

## ESP32 Code (Arduino with `PubSubClient`)

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define LEDPIN 2

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_RPI_IP";  // e.g., 192.168.1.100

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (String(topic) == "esp32/led") {
    if (msg == "on") digitalWrite(LEDPIN, HIGH);
    else if (msg == "off") digitalWrite(LEDPIN, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("esp32/led");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    String payload = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}";
    client.publish("esp32/dht", payload.c_str());
  }

  delay(5000);
}
```

---

## Raspberry Pi: Mosquitto Running with **Anonymous Access Enabled**

Check with:

```bash
cat /etc/mosquitto/mosquitto.conf
```

Key lines (insecure config):

```conf
listener 1883
allow_anonymous true
```

Restart broker:

```bash
sudo systemctl restart mosquitto
```

---

## Attack Demonstration (from Attacker Laptop or RPi Itself)

### 1. **Subscribe to all data (esp32/dht)**:

```bash
mosquitto_sub -h <RPI_IP> -t "#" -v
```

Shows:

```
esp32/dht {"temp":27.8,"hum":55.1}
```

### 2. **Turn on the ESP32 LED remotely (unauthorized control)**:

```bash
mosquitto_pub -h <RPI_IP> -t esp32/led -m "on"
```

**LED on ESP32 turns on**

Anyone on the same network can control the device and read sensor data!

---

##  Remediation: Disable Anonymous Access

### Step 1: Create User + Password

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd espuser
```

You'll be prompted to enter a password.

### Step 2: Update `mosquitto.conf` securely

```conf
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd
```

Restart Mosquitto:

```bash
sudo systemctl restart mosquitto
```

---

## Update ESP32 Code to Use Credentials

Replace:

```cpp
client.connect("ESP32Client")
```

With:

```cpp
client.connect("ESP32Client", "espuser", "your_password")
```

---

##  Test the Fix

### From attacker device:

```bash
mosquitto_pub -h <RPI_IP> -t esp32/led -m "on"
```

 **Fails** with:

```
Connection Refused: not authorised.
```

### Authorized publish:

```bash
mosquitto_pub -u espuser -P your_password -h <RPI_IP> -t esp32/led -m "on"
```

 Works as expected

---

## 📋 Summary for Workshop

| 🔍                | Item                                                    |
| ----------------- | ------------------------------------------------------- |
| **Vulnerability** | Anonymous access to MQTT broker                         |
| **Impact**        | Unauthorized data access and device control             |
| **Demo**          | Publish/subscribe without credentials                   |
| **Fix**           | Disable anonymous access, set username/password         |
| **Bonus**         | Show Mosquitto logs: `/var/log/mosquitto/mosquitto.log` |

---


