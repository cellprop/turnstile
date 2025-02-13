#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ctime>
#include <ctype.h>  // For isalnum function

// ✅ Wi-Fi and MQTT settings (from esp_final)
const char* ssid = "nWO";
const char* pass = "Cp#super123";
const char* mqtt_server = "192.168.68.106";
const int mqtt_port = 1884;

// ✅ MQTT Topics
const char* publish_topic = "turnstile_publish";
const char* subscribe_topic = "turnstile_subscribe";

// ✅ GPIO for SoftwareSerial
#define UART1_RX_PIN 5  // D1
#define UART1_TX_PIN 4  // D2

// ✅ UART Buffer Management (14 bytes total: RFID + Entry/Exit + Turnstile ID)
volatile char uart1_buffer[14];  
volatile int uart1_index = 0;
volatile bool uart1_data_received = false;
String error_code = "21";

// ✅ MQTT and Wi-Fi Objects
SoftwareSerial uart1(UART1_RX_PIN, UART1_TX_PIN);  
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  

const char* portId = "P01A";  
#define BAUD_RATE 9600

// ✅ Function Prototypes
void setup_wifi();
void reconnect();
void publishMessage(const char* rfid, const char* command, const char* turnstile_id);
void callback(char* topic, byte* payload, unsigned int length);
void sendToUART(Stream &uart, const char* message);
String getTimestamp();
void uart_rx_handler(); 

// ✅ Setup Function
void setup() {
    Serial.begin(BAUD_RATE);  
    uart1.begin(BAUD_RATE);   

    Serial.println("[INFO] Initializing ESP8266 Turnstile System...");

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    timeClient.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); 
    Serial.println("[INFO] System Initialized Successfully.");
}

// ✅ Main Loop (Maintains MQTT Connection and UART)
void loop() {
    if (!client.connected()) {
        reconnect();  // ✅ Ensuring stable connection before any MQTT action
    }
    client.loop();
    timeClient.update();

    // ✅ Continuously check for UART data
    uart_rx_handler();  
}

// ✅ UART Reception Handler
void uart_rx_handler() {
    while (uart1.available() > 0) {
        char c = uart1.read();
        Serial.println("Reading UART Byte...");

        if (uart1_index < sizeof(uart1_buffer)) {  
            uart1_buffer[uart1_index++] = c;
        }

        // ✅ If 14 bytes are received, process data
        if (uart1_index >= sizeof(uart1_buffer)) {
            uart1_buffer[13] = '\0';  // Null terminate for safety
            uart1_data_received = true;
            uart1_index = 0;  

            Serial.println("[DEBUG] Complete 14-byte UART Message Received!");
            
            // ✅ Parsing the 14-byte UART data
            char rfid[13] = {0};         
            char command[2] = {0};       
            char turnstile_id[2] = {0};  

            strncpy(rfid, (const char*)uart1_buffer, 12);  
            command[0] = uart1_buffer[12];    
            turnstile_id[0] = uart1_buffer[13];  

            Serial.print("[USART RX] RFID: "); Serial.println(rfid);
            Serial.print("[USART RX] Command: "); Serial.println(command);
            Serial.print("[USART RX] Turnstile ID: "); Serial.println(turnstile_id);

            // ✅ Data validation before publishing
            if ((command[0] == '1' || command[0] == '2') && isalnum(turnstile_id[0])) {
                Serial.println("[DEBUG] Data validated. Publishing...");
                publishMessage(rfid, command, turnstile_id);
            } else {
                Serial.println("[ERROR] Invalid data received, skipping publish.");
                sendToUART(uart1, error_code.c_str());
            }
        }
    }
}

// ✅ Wi-Fi Setup Function
void setup_wifi() {
    Serial.print("[INFO] Connecting to Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[INFO] WiFi Connected Successfully!");
    Serial.print("[INFO] IP Address: ");
    Serial.println(WiFi.localIP());
}

// ✅ MQTT Reconnection (Using mqtt_publish Logic)
void reconnect() {
    while (!client.connected()) {
        Serial.print("[DEBUG] Attempting MQTT Connection...");
        if (client.connect("ESP8266Client")) {
            Serial.println("[INFO] MQTT Connected Successfully.");
            client.subscribe(subscribe_topic);  
        } else {
            Serial.print("[ERROR] Connection failed, state: ");
            Serial.println(client.state());
            delay(5000);  
        }
    }
}

// ✅ MQTT Publishing (Technique from `mqtt_publish`)
void publishMessage(const char* rfid, const char* command, const char* turnstile_id) {
    StaticJsonDocument<200> doc;
    doc["RFID"] = rfid;
    doc["entry"] = command;
    doc["turnstileID"] = turnstile_id;
    doc["timeStamp"] = getTimestamp();
    doc["portId"] = portId;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    // ✅ Consistent MQTT Publish Technique
    if (client.publish(publish_topic, jsonBuffer)) {
        Serial.println("[MQTT TX] Data Published Successfully!");
    } else {
        Serial.println("[ERROR] Failed to publish. Sending error via UART.");
        sendToUART(uart1, error_code.c_str());
    }
}

// ✅ MQTT Subscription Callback (Technique from `mqtt_subscribe`)
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("[MQTT RX] Message received on topic: ");
    Serial.println(topic);

    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("[MQTT RX] Message Content: ");
    Serial.println(message);

    // ✅ Forward the message to UART after receiving on MQTT
    sendToUART(uart1, message.c_str());
}

// ✅ Send Data to UART
void sendToUART(Stream &uart, const char* message) {
    uart.write(message);
    Serial.print("[USART TX] Sent to UART: ");
    Serial.println(message);
}

// ✅ IST Timestamp Handling
String getTimestamp() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    epochTime += 19800;  
    struct tm *timeInfo;
    time_t rawTime = epochTime;
    timeInfo = gmtime(&rawTime);  

    char timeBuffer[20];
    sprintf(timeBuffer, "%02d:%02d:%02d IST", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    return String(timeBuffer);
}