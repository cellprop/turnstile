#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ctype.h> // Needed for isalnum function

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

// NTP Client settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC timezone, updates every minute

const char* portId = "P01A";  // Static port ID
#define BAUD_RATE 9600

// UART Buffers
char uart1_buffer[14];  // Exactly 14 bytes for RFID (12) + entry/exit (1) + turnstile ID (1)
bool uart1_data_received = false;

// Function prototypes
void setup_wifi();
void reconnect();
void publishMessage(const char* rfid, const char* command, const char* turnstile_id);
void callback(char* topic, byte* payload, unsigned int length);
void processUART();
void sendToUART(Stream &uart, const char* message);
String getTimestamp();

void setup() {
  Serial.begin(BAUD_RATE);  // Initialize UART0
  uart1.begin(BAUD_RATE);   // Initialize UART1
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Start the NTP client
  timeClient.begin();

  // Set up onboard LED for visual indication
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn off LED initially
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Update the time from NTP server
  timeClient.update();

  // Process UART1
  processUART();
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
void publishMessage(const char* rfid, const char* command, const char* turnstile_id) {
  if (client.connected()) {
    StaticJsonDocument<200> doc;
    doc["RFID"] = rfid;
    doc["entry"] = command;
    doc["turnstileID"] = turnstile_id;
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

  // Send the received command back via UART
  sendToUART(uart1, message.c_str());
}

// Process UART data
void processUART() {
    static int index = 0;

    while (uart1.available() > 0) {
        delay(500); // Small delay before reading UART
        char c = uart1.read();
        uart1_buffer[index++] = c;

        if (index >= 16) {  // When all 16 bytes are received (14 RFID + 1 Entry/Exit + 1 Turnstile ID)
            uart1_buffer[index] = '\0';  // Null-terminate
            uart1_data_received = true;
            index = 0;  // Reset index for next message

            // Split data
            char rfid[15] = {0};  // 14 bytes for RFID + 1 null-terminator
            char command[2] = {0};  // 1 byte for entry/exit + 1 null-terminator
            char turnstile_id[2] = {0};  // 1 byte for turnstile ID + 1 null-terminator

            strncpy(rfid, uart1_buffer, 14);
            command[0] = uart1_buffer[14];
            turnstile_id[0] = uart1_buffer[15];

            // Validate received data
            if ((command[0] == '1' || command[0] == '2') && isalnum(turnstile_id[0])) {
                publishMessage(rfid, command, turnstile_id);
            } else {
                Serial.println("Error: Invalid entry/exit value received. Must be '1' or '2'.");
            }
        }
    }
}

// Send message to UART
void sendToUART(Stream &uart, const char* message) {
  Serial.print("Sending to UART: ");
  Serial.println(message);
  uart.write(message);
}

// Get current timestamp and convert UTC to IST
String getTimestamp() {
    // Fetch the current UTC time from the NTP client
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();  // Get epoch time (seconds since Jan 1, 1970)
    
    // Convert UTC to IST (UTC + 5 hours 30 minutes = UTC + 19800 seconds)
    epochTime += 19800;  // Adding 5 hours 30 minutes in seconds

    // Break down epoch time into date components
    int hours = (epochTime % 86400L) / 3600;  // Hours since midnight
    int minutes = (epochTime % 3600) / 60;
    int seconds = epochTime % 60;

    // Return formatted IST time as a string
    char timeBuffer[20];
    sprintf(timeBuffer, "%02d:%02d:%02d IST", hours, minutes, seconds);
    return String(timeBuffer);
}