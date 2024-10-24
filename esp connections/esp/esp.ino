#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Replace with your network and MQTT server details
const char* ssid = "Cellprop";     // Replace with your Wi-Fi SSID
const char* pass = "Cp#deco123";  // Replace with your Wi-Fi password
const char* mqtt_server = "192.168.68.138";  // Replace with your MQTT server address

WiFiClient espClient;
PubSubClient client(espClient);

// Define topics for publishing and subscribing
const char* publish_topic = "turnstile_publish";
const char* subscribe_topic = "turnstile_subscribe";

// Buffer to store incoming UART data
char uart_data[256];
bool uart_data_received = false;

// Function prototypes
void setup_wifi();
void reconnect();
void publishMessage(const char* message);
void callback(char* topic, byte* payload, unsigned int length);
void readFromUART();
void transmitToUART(const char* message);

void setup() {
  Serial.begin(9600);  // Initialize UART communication
  setup_wifi();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);
}

void loop() {
  // Step 1: Wait for data via UART
  readFromUART();

  // Step 2: If data is received, handle the MQTT actions
  if (uart_data_received) {
    // Ensure the client is connected to the MQTT broker
    if (!client.connected()) {
      reconnect();
    }

    // Publish the received UART data to the MQTT topic
    publishMessage(uart_data);

    // Process incoming MQTT messages (handled by callback)
    client.loop();

    // Reset the UART data received flag after processing
    uart_data_received = false;
  }
}

// Function to connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// Function to reconnect to the MQTT broker
void reconnect() {
  // Loop until the client is connected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(subscribe_topic);  // Subscribe to the topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

// Function to publish a message to the MQTT topic
void publishMessage(const char* message) {
  if (client.connected()) {
    client.publish(publish_topic, message);
    Serial.print("Message published: ");
    Serial.println(message);
  }
}

// Callback function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Extract the message and check for '1' or '0'
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Step 4: Transmit '1' or '0' received via MQTT back via UART
  if (message == "1" || message == "0") {
    transmitToUART(message.c_str());
  } else {
    Serial.println("Received message is not '1' or '0'. Ignoring.");
  }
}

// Function to read data from UART
void readFromUART() {
  if (Serial.available() > 0) {
    int index = 0;
    while (Serial.available() > 0 && index < sizeof(uart_data) - 1) {
      uart_data[index++] = Serial.read();
      delay(10);  // Add a small delay for stability
    }
    uart_data[index] = '\0';  // Null-terminate the string
    uart_data_received = true;
    Serial.print("Data received via UART: ");
    Serial.println(uart_data);
  }
}

// Function to transmit data via UART
void transmitToUART(const char* message) {
  Serial.print("Sending back via UART: ");
  Serial.println(message);
  Serial.write(message);  // Send the message over UART as raw data
}