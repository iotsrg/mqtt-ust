Steps to setup MQTT Client: (Using ESP32)


* Read temperature and humidity from the DHT11 sensor.
* Included a counter that increments with each MQTT publish.
* Send both the sensor values and the counter in the payload.


 Full Arduino Code: ESP32 + DHT11 + MQTT + Counter

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "testing";
const char* password = "00000000";

// MQTT broker IP
const char* mqtt_server = "192.168.1.102";

// DHT configuration
#define DHTPIN 4          // GPIO4 (D2 on board)
#define DHTTYPE DHT11     // DHT11 Sensor
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
int counter = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (message == "on") {
    digitalWrite(2, HIGH);
  } else if (message == "off") {
    digitalWrite(2, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    if (client.connect("ESP32Client")) {
      Serial.println("connected ✅");
      client.subscribe("/admin/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);  // Built-in LED
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed.");
    return;
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish every 5 seconds
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("❌ Failed to read from DHT sensor!");
      return;
    }

    counter++;
    String payload = "Count: " + String(counter) +
                     ", Temp: " + String(temp, 1) + "°C" +
                     ", Hum: " + String(hum, 1) + "%";

    Serial.println("📤 Publishing: " + payload);
    client.publish("/esp32/sensor", payload.c_str());
  }
}
```

Notes:

* It publishes to the topic /esp32/sensor.
* You can subscribe using mosquitto\_sub on your PC:

```bash
mosquitto_sub -h 192.168.1.102 -t "/esp32/sensor"
```

