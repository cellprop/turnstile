# Bi-Directional RFID Turnstile

This repository contains the electronic control system for a bi-directional turnstile with RFID authentication for entry and exit.

## Project Overview

This turnstile system includes:
- RFID authentication for both entry and exit
- Bi-directional movement control
- Limit switches for door position sensing
- Motor control for smooth operation
- MQTT communication for remote monitoring and control

## Repository Structure

```
.
├── README.md
├── src/
│   ├── rfid_sensor/
│   ├── limit_switches/
│   ├── motor_control/
│   └── main_controller/
├── schematics/
├── docs/
└── tests/
```

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/cellprop/turnstile.git
   ```
2. Install the required dependencies.

## Hardware Requirements
- RFID sensor module
- Limit switches
- Motor and motor driver
- LED Matrix
- Microcontroller (STM32 Board)

## MQTT Connection Error Codes

When setting up MQTT communication, you may encounter connection issues. Below are the possible return codes (`rc` values) when an MQTT connection fails and their meanings:

- **`rc = -4`**: **MQTT_CONNECTION_TIMEOUT**  
  The connection attempt timed out. This could indicate network issues, a firewall blocking the connection, or the broker being down.

- **`rc = -3`**: **MQTT_CONNECTION_LOST**  
  The connection was lost after being established. This can happen due to network interruptions or an unexpected broker disconnection.

- **`rc = -2`**: **MQTT_CONNECT_FAILED**  
  The client failed to establish a connection, which might be caused by incorrect server details or the broker rejecting the connection.

- **`rc = -1`**: **MQTT_DISCONNECTED**  
  The client is disconnected from the broker. This could be due to a manual disconnect or failure to maintain the connection.

- **`rc = 0`**: **MQTT_CONNECTED**  
  The client is successfully connected to the broker.

- **`rc = 1`**: **Connection Refused, Unacceptable Protocol Version**  
  The MQTT broker does not support the protocol version used by the client.

- **`rc = 2`**: **Connection Refused, Identifier Rejected**  
  The client identifier is invalid. Ensure it is not empty or duplicated.

- **`rc = 3`**: **Connection Refused, Server Unavailable**  
  The broker is unavailable, possibly due to maintenance, overload, or connection limits.

- **`rc = 4`**: **Connection Refused, Bad Username or Password**  
  The credentials provided by the client are incorrect.

- **`rc = 5`**: **Connection Refused, Not Authorized**  
  The client lacks permission to connect. Check for incorrect credentials or access restrictions.

These error codes can help diagnose issues during MQTT setup and ensure a stable connection between the turnstile system and the MQTT broker.


