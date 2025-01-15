import paho.mqtt.client as mqtt
import json
import time
from datetime import datetime
from gpiozero import Button, DigitalOutputDevice
from threading import Thread
from queue import Queue
import logging

# Setup logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

# Load configuration
def load_config(filename="config.json"):
    try:
        with open(filename, "r") as file:
            return json.load(file)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        logger.error(f"Error loading config: {e}")
        raise

config = load_config()

# Configuration details
MQTT_SERVER = config["mqtt_server"]
MQTT_PORT = config["mqtt_port"]
PUBLISH_TOPIC = "turnstile_publish"
SUBSCRIBE_TOPIC = "turnstile_subscribe"
PORT_ID = config["port_id"]

# GPIO pin assignments
UART1_RX_PIN = config["uart1_rx_pin"]
UART1_TX_PIN = config["uart1_tx_pin"]
UART2_RX_PIN = config["uart2_rx_pin"]
UART2_TX_PIN = config["uart2_tx_pin"]

BAUD_RATE = config["baud_rate"]
BIT_DELAY = 1 / BAUD_RATE

# Buffers for UART communication
uart_buffer_1 = Queue()
uart_buffer_2 = Queue()

# GPIO setup
uart1_rx = Button(UART1_RX_PIN, pull_up=False)
uart1_tx = DigitalOutputDevice(UART1_TX_PIN, active_high=True)
uart2_rx = Button(UART2_RX_PIN, pull_up=False)
uart2_tx = DigitalOutputDevice(UART2_TX_PIN, active_high=True)

# Initialize MQTT Client with version 2
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# MQTT Callback Functions
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        logger.info(f"✅ Connected to MQTT Broker at {MQTT_SERVER}:{MQTT_PORT}")
        client.subscribe(SUBSCRIBE_TOPIC)
    elif rc == 5:
        logger.error("❌ Connection refused: Authentication failed!")
    else:
        logger.error(f"❌ Failed to connect to MQTT Broker. Return code: {rc}")

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        received_value = int(payload.get('message', 0))  # Default to 0 if 'message' is missing
        
        # Validate the received message
        if 1 <= received_value <= 10:
            transmit_to_uart(1, str(received_value))
            transmit_to_uart(2, str(received_value))
        elif received_value > 10:
            recent = received_value % 10
            transmit_to_uart(1, str(recent))
            transmit_to_uart(2, str(recent))
        else:
            logger.warning("⚠️ Received invalid message value.")
    except (ValueError, json.JSONDecodeError) as e:
        logger.error(f"Error processing MQTT message: {e}")

def publish_message(client, uart_id, command, turnstile_id):
    payload = {
        "RFID": uart_id,
        "entry": command,
        "turnstileID": turnstile_id,
        "timeStamp": get_timestamp(),
        "portId": PORT_ID
    }
    try:
        result = client.publish(PUBLISH_TOPIC, json.dumps(payload))
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            logger.info(f"📤 Successfully published message from {uart_id}")
        else:
            logger.error(f"❌ Failed to publish message from {uart_id}")
    except Exception as e:
        logger.error(f"❌ Error while publishing MQTT message: {e}")

# UART RX Handler
def uart_interrupt_handler(uart_rx, buffer, client, uart_id):
    while True:
        try:
            uart_rx.wait_for_active()
            time.sleep(BIT_DELAY / 2)
            data = 0
            for i in range(8):
                time.sleep(BIT_DELAY)
                data |= uart_rx.is_active << i
            stop_bit = uart_rx.is_active
            if not stop_bit:
                logger.warning(f"⚠️ Framing error detected on {uart_id}")
                continue
            buffer.put(chr(data))
            if buffer.qsize() >= 2:
                command = buffer.get()
                turnstile_id = buffer.get()
                if command.isalnum() and turnstile_id.isalnum():
                    publish_message(client, uart_id, command, turnstile_id)
                else:
                    logger.warning(f"⚠️ Invalid UART data received on {uart_id}")
        except Exception as e:
            logger.error(f"❌ Error in {uart_id} RX handler: {e}")

# UART Transmission
def transmit_to_uart(uart_id, command):
    tx = uart1_tx if uart_id == 1 else uart2_tx
    turnstile_id = str(uart_id)
    full_message = f"{command}{turnstile_id}"
    logger.info(f"🔗 Sending via UART{uart_id}: {full_message}")
    for char in full_message:
        bits = f'1{bin(ord(char))[2:].zfill(8)}0'  # Start bit + Data bits + Stop bit
        for bit in bits:
            tx.value = int(bit)
            time.sleep(BIT_DELAY)

# Timestamp Generator
def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Improved MQTT Reconnection Logic
def mqtt_reconnect():
    while True:
        try:
            client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
            logger.info("✅ Reconnected to MQTT Broker.")
            client.loop_start()
            break
        except Exception as e:
            logger.error(f"❌ Reconnection failed: {e}")
            time.sleep(5)

# Assign MQTT Callbacks
client.on_connect = on_connect
client.on_message = on_message

# Main Function
def main():
    try:
        client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
        client.loop_start()
    except Exception as e:
        logger.error(f"❌ Initial connection failed: {e}")
        mqtt_reconnect()

    # Start UART threads
    uart_thread_1 = Thread(target=uart_interrupt_handler, args=(uart1_rx, uart_buffer_1, client, "UART1"), daemon=True)
    uart_thread_2 = Thread(target=uart_interrupt_handler, args=(uart2_rx, uart_buffer_2, client, "UART2"), daemon=True)
    uart_thread_1.start()
    uart_thread_2.start()

    try:
        while True:
            time.sleep(0.5)
    except KeyboardInterrupt:
        logger.info("🛑 Shutting down gracefully...")
        client.loop_stop()
        client.disconnect()
        uart1_tx.close()
        uart2_tx.close()

if __name__ == "__main__":
    main()
