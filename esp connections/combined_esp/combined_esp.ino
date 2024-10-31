#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Replace with your network and MQTT server details
const char* ssid = "nWO";     // Replace with your Wi-Fi SSID
const char* pass = "Cp#super123";  // Replace with your Wi-Fi password
const char* mqtt_server = "192.168.68.106";  // Replace with your MQTT server address
int recent;

WiFiClient espClient;
PubSubClient client(espClient);

// Define topics for publishing and subscribing
const char* publish_topic = "turnstile_publish";
const char* subscribe_topic = "turnstile_subscribe";

// Define static port ID
const char* portId = "P01A"; // Set your port ID here

// Buffer to store incoming UART data
char uart_data[256];
bool uart_data_received = false;

// Function prototypes
void setup_wifi();
void reconnect();
void publishMessage(const char* rfid, int entry);
void callback(char* topic, byte* payload, unsigned int length);
void readFromUART();
void transmitToUART(const char* message);
String getTimestamp();

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

    // Extract RFID and entry value from uart_data
    String rfid = String(uart_data).substring(0, 12); // First 12 characters
    int entry = String(uart_data[12]).toInt(); // 13th character

    // Publish the received data as JSON
    publishMessage(rfid.c_str(), entry);

    // Listen for incoming MQTT messages (handled by callback)
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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(subscribe_topic);  // Subscribe to the topic
      Serial.print("Subscribed to topic: ");
      Serial.println(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

// Function to publish JSON message to the MQTT topic
void publishMessage(const char* rfid, int entry) {
  if (client.connected()) {
    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["RFID"] = rfid;
    doc["timeStamp"] = getTimestamp();
    doc["entry"] = entry;
    doc["portId"] = portId;

    // Serialize JSON to string
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    // Publish JSON message
    client.publish(publish_topic, jsonBuffer);
    Serial.print("Message published: ");
    Serial.println(jsonBuffer);
  }
}

// Callback function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Extract the message
  String message;
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println(message);

  // Step 4: Transmit the received numerical value (1-10) back via UART
  int received_value = message.toInt();
  if (received_value >= 1 && received_value <= 10) {
    transmitToUART(String(received_value).c_str());
  }
  else if (received_value > 10){
    recent = received_value % 10;
    transmitToUART(String(recent).c_str());    
  } 

}

// Function to read data from UART
void readFromUART() {
  if (Serial.available() >= 13) { // Check if we have at least 13 characters
    int index = 0;
    while (Serial.available() > 0 && index < 13) {
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

// Function to get a timestamp in the format "YYYY-MM-DD HH:MM:SS"
String getTimestamp() {
  // Example timestamp (replace with RTC or NTP server as needed)
  String timestamp = "2024-10-24 12:34:56"; // Use a method to get real-time data if needed
  return timestamp;
}