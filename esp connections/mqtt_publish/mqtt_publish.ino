#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Cellprop";     // Replace with your Wi-Fi SSID
const char* pass = "Cp#deco123";  // Replace with your Wi-Fi password
const char* mqtt_server = "192.168.68.138";  // Replace with your MQTT server address

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1884);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("turnstile", "Shivam is absent");
  delay(2000);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to ");
  Serial.println(ssid); 

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or publish as needed
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}