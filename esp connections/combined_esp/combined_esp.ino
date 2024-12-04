#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// Wi-Fi and MQTT settings
const char* ssid = "nWO";                // Wi-Fi SSID
const char* pass = "Cp#super123";        // Wi-Fi Password
const char* mqtt_server = "192.168.68.106";
const int mqtt_port = 1884;

// Topics
const char* publish_topic = "turnstile_publish";
const char* subscribe_topic = "turnstile_subscribe";

// GPIO for SoftwareSerial
#define UART1_RX_PIN 5  // D1
#define UART1_TX_PIN 4  // D2

SoftwareSerial uart1(UART1_RX_PIN, UART1_TX_PIN);  // RX, TX for second UART
WiFiClient espClient;
PubSubClient client(espClient);

const char* portId = "P01A";  // Static port ID
#define BAUD_RATE 9600

// UART Buffers
char uart1_buffer[256];
char uart0_buffer[256];
bool uart1_data_received = false;
bool uart0_data_received = false;

// Function prototypes
void setup_wifi();
void reconnect();
void publishMessage(int uart_id, const char* command, const char* turnstile_id);
void callback(char* topic, byte* payload, unsigned int length);
void processUART(SoftwareSerial &uart, char* buffer, bool &data_flag, int uart_id);
void sendToUART(SoftwareSerial &uart, const char* message);
String getTimestamp();

void setup() {
  Serial.begin(BAUD_RATE);  // Initialize UART0
  uart1.begin(BAUD_RATE);   // Initialize UART1
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Process UART0
  processUART(Serial, uart0_buffer, uart0_data_received, 0);

  // Process UART1
  processUART(uart1, uart1_buffer, uart1_data_received, 1);
}

// Connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// Reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Publish message to MQTT
void publishMessage(int uart_id, const char* command, const char* turnstile_id) {
  if (client.connected()) {
    StaticJsonDocument<200> doc;
    doc["UART_ID"] = uart_id;
    doc["Command"] = command;
    doc["TurnstileID"] = turnstile_id;
    doc["timeStamp"] = getTimestamp();
    doc["portId"] = portId;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    client.publish(publish_topic, jsonBuffer);

    Serial.print("Published to MQTT: ");
    Serial.println(jsonBuffer);
  }
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Process message (assume it's a command)
  int received_value = message.toInt();
  if (received_value >= 1 && received_value <= 10) {
    sendToUART(Serial, String(received_value).c_str());
    sendToUART(uart1, String(received_value).c_str());
  } else if (received_value > 10) {
    int recent = received_value % 10;
    sendToUART(Serial, String(recent).c_str());
    sendToUART(uart1, String(recent).c_str());
  }
}

// Process UART input
void processUART(SoftwareSerial &uart, char* buffer, bool &data_flag, int uart_id) {
  if (uart.available() >= 2) {  // Minimum data length for command + turnstile ID
    int index = 0;
    while (uart.available() > 0 && index < sizeof(buffer) - 1) {
      buffer[index++] = uart.read();
      delay(10);  // Stabilize input
    }
    buffer[index] = '\0';
    data_flag = true;

    Serial.print("Data received on UART");
    Serial.print(uart_id);
    Serial.print(": ");
    Serial.println(buffer);

    // Split command and ID
    if (index >= 2) {
      String command = String(buffer[0]);
      String turnstile_id = String(buffer[1]);

      // Validate data
      if (command.isAlnum() && turnstile_id.isAlnum()) {
        publishMessage(uart_id, command.c_str(), turnstile_id.c_str());
      } else {
        Serial.print("Invalid data on UART");
        Serial.println(uart_id);
      }
    }
  }
}

// Send data via UART
void sendToUART(SoftwareSerial &uart, const char* message) {
  Serial.print("Sending to UART: ");
  Serial.println(message);
  uart.write(message);
}

// Get current timestamp
String getTimestamp() {
  return "2024-12-04 12:00:00";  // Replace with RTC or NTP logic
}