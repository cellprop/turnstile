#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <ctype.h>  // For isalnum function

// ✅ GPIO for SoftwareSerial
#define UART1_RX_PIN 5  // D1
#define UART1_TX_PIN 4  // D2

// ✅ Initialize Software Serial for UART Communication
SoftwareSerial uart1(UART1_RX_PIN, UART1_TX_PIN);  // RX, TX for second UART

// ✅ UART Buffer Configuration
volatile char uart1_buffer[16];  // Buffer size: 14 bytes for data + 2 for safety
volatile bool uart1_data_received = false;
volatile int uart1_index = 0;

#define BAUD_RATE 9600  // UART Communication Speed

// ✅ Function Prototypes
void IRAM_ATTR uartISR();
void printReceivedData();

// ✅ Setup Function (UART Initialization Only)
void setup() {
    Serial.begin(BAUD_RATE);   // Primary Serial Monitor Output
    uart1.begin(BAUD_RATE);    // Software Serial for Data Reception

    Serial.println("[INFO] ESP8266 UART Reception Only - Initializing...");

    // ✅ Attach Interrupt for UART RX Pin (FALLING EDGE)
    attachInterrupt(digitalPinToInterrupt(UART1_RX_PIN), uartISR, FALLING);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);  // LED off initially
}

// ✅ Main Loop (Only Receives and Prints UART Data)
void loop() {
    if (uart1_data_received) {
        uart1_data_received = false;  // Reset flag
        printReceivedData();          // Print received data after complete message
    }
}

// ✅ Corrected ISR for Receiving UART Data
void IRAM_ATTR uartISR() {
    char c = uart1.read();  
    uart1_buffer[uart1_index++] = c;

    // ✅ Buffer Overflow Prevention & Message Completion Handling
    if (uart1_index >= sizeof(uart1_buffer)) {
        uart1_buffer[15] = '\0';  // Null-terminate for safety
        uart1_data_received = true;
        uart1_index = 0;  
        Serial.println("[USART RX] Data reception complete.");
    }
}

// ✅ Function to Print Received UART Data
// ✅ Function to Print Received UART Data (Fixed)
void printReceivedData() {
    Serial.println("[USART RX] Complete UART message received!");

    // ✅ Print received data as a readable string (Fixed)
    Serial.print("[USART RX] Raw Data: ");
    Serial.println((const char*)uart1_buffer);  // Fixed Casting Issue

    // ✅ Hexadecimal View for Debugging (Optional)
    Serial.print("[HEX Data] ");
    for (int i = 0; i < 16; i++) {
       // Serial.printf("%02X ", (unsigned char)uart1_buffer[i]);
       
        
    }
    Serial.println();
}