import paho.mqtt.client as mqtt
import json
import time
from datetime import datetime
from gpiozero import Button, DigitalOutputDevice
from threading import Thread, Event

# Configuration details
MQTT_SERVER = "192.168.68.106"
MQTT_PORT = 1884
PUBLISH_TOPIC = "turnstile_publish"
SUBSCRIBE_TOPIC = "turnstile_subscribe"
PORT_ID = "P01A"

# GPIO pin assignments for software UARTs (BCM numbering)
UART1_RX_PIN = 17  # GPIO 17 for UART1 RX
UART1_TX_PIN = 18  # GPIO 18 for UART1 TX
UART2_RX_PIN = 27  # GPIO 27 for UART2 RX
UART2_TX_PIN = 22  # GPIO 22 for UART2 TX

BAUD_RATE = 9600  # UART baud rate
BIT_DELAY = 1 / BAUD_RATE  # Delay per bit in seconds

# Event for simulating interrupt-driven UART reception
uart_interrupt_event_1 = Event()
uart_interrupt_event_2 = Event()
uart_buffer_1 = []
uart_buffer_2 = []

# GPIO setup (using BCM pin numbering)
uart1_rx = Button(UART1_RX_PIN, pull_up=False)  # RX pin for UART1
uart1_tx = DigitalOutputDevice(UART1_TX_PIN, active_high=True)  # TX pin for UART1
uart2_rx = Button(UART2_RX_PIN, pull_up=False)  # RX pin for UART2
uart2_tx = DigitalOutputDevice(UART2_TX_PIN, active_high=True)  # TX pin for UART2

# MQTT callback functions
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        client.subscribe(SUBSCRIBE_TOPIC)
        print(f"Subscribed to topic: {SUBSCRIBE_TOPIC}")
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload.decode()}")

    try:
        # Extract the received value and send it back via both UARTs
        received_value = int(msg.payload.decode())
        if 1 <= received_value <= 10:
            transmit_to_uart(1, str(received_value))
            transmit_to_uart(2, str(received_value))
        elif received_value > 10:
            recent = received_value % 10
            transmit_to_uart(1, str(recent))
            transmit_to_uart(2, str(recent))
    except ValueError:
        print("Invalid message format")

# Function to publish JSON message
def publish_message(client, uart_id, command, turnstile_id):
    if client.is_connected():
        # Create JSON payload
        payload = {
            "UART_ID": uart_id,
            "Command": command,
            "TurnstileID": turnstile_id,
            "timeStamp": get_timestamp(),
            "portId": PORT_ID
        }
        # Publish to MQTT broker
        client.publish(PUBLISH_TOPIC, json.dumps(payload))
        print(f"Message published from {uart_id}: {json.dumps(payload)}")

# Function to handle UART RX for UART1
def uart_interrupt_handler_1():
    global uart_buffer_1
    while True:
        # Wait for start bit (falling edge on RX)
        uart1_rx.wait_for_active()
        time.sleep(BIT_DELAY / 2)  # Wait for the middle of the start bit
        data = 0
        for i in range(8):  # Read 8 data bits
            time.sleep(BIT_DELAY)
            data |= uart1_rx.is_active << i
        uart_buffer_1.append(chr(data))
        if len(uart_buffer_1) == 2:  # Expect exactly 2 characters: CMD + ID
            command = uart_buffer_1[0]
            turnstile_id = uart_buffer_1[1]
            uart_buffer_1 = []  # Reset buffer
            print(f"Received from UART1: Command={command}, TurnstileID={turnstile_id}")
            publish_message(None, "UART1", command, turnstile_id)
            uart_interrupt_event_1.set()

# Function to handle UART RX for UART2
def uart_interrupt_handler_2():
    global uart_buffer_2
    while True:
        # Wait for start bit (falling edge on RX)
        uart2_rx.wait_for_active()
        time.sleep(BIT_DELAY / 2)  # Wait for the middle of the start bit
        data = 0
        for i in range(8):  # Read 8 data bits
            time.sleep(BIT_DELAY)
            data |= uart2_rx.is_active << i
        uart_buffer_2.append(chr(data))
        if len(uart_buffer_2) == 2:  # Expect exactly 2 characters: CMD + ID
            command = uart_buffer_2[0]
            turnstile_id = uart_buffer_2[1]
            uart_buffer_2 = []  # Reset buffer
            print(f"Received from UART2: Command={command}, TurnstileID={turnstile_id}")
            publish_message(None, "UART2", command, turnstile_id)
            uart_interrupt_event_2.set()

# Function to send data via UART TX
def transmit_to_uart(uart_id, command):
    tx = uart1_tx if uart_id == 1 else uart2_tx
    turnstile_id = str(uart_id)  # Set turnstile_id as UART ID
    full_message = f"{command}{turnstile_id}"  # Format: CMDID (e.g., "12")
    print(f"Sending via UART{uart_id}: {full_message}")
    for char in full_message:
        # Start bit
        tx.off()
        time.sleep(BIT_DELAY)
        # Data bits
        for i in range(8):
            bit = (ord(char) >> i) & 1
            tx.value = bit
            time.sleep(BIT_DELAY)
        # Stop bit
        tx.on()
        time.sleep(BIT_DELAY)

# Function to get a timestamp
def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Main execution
def main():
    global uart_buffer_1, uart_buffer_2

    # Initialize MQTT client
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Connect to MQTT broker
    try:
        client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
    except Exception as e:
        print(f"Failed to connect to MQTT Broker: {e}")
        return

    client.loop_start()

    # Start the UART interrupt handlers in separate threads
    uart_thread_1 = Thread(target=uart_interrupt_handler_1, daemon=True)
    uart_thread_2 = Thread(target=uart_interrupt_handler_2, daemon=True)
    uart_thread_1.start()
    uart_thread_2.start()

    while True:
        # Main loop runs indefinitely
        time.sleep(0.1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting program...")