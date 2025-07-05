#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Mr-IoT";
const char* password = "v33ru@123";
const char* mqtt_server = "192.168.0.100";  // Adjust as needed

WiFiClient espClient;
PubSubClient client(espClient);

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
    Serial.print("Attempting MQTT connection...");
    if (client.connect("esp32client")) {
      Serial.println("connected");
      client.subscribe("/admin/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    retry_count++;
    if (retry_count > 20) {
      Serial.println("\n❌ Failed to connect to WiFi. Check SSID/Password/Range.");
      return;
    }
  }

  Serial.println();
  Serial.println("✅ WiFi connected.");
  Serial.print("📡 IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
