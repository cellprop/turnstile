import struct
import serial
import time
import paho.mqtt.client as mqtt
import json
from datetime import datetime
from threading import Thread
from queue import Queue

# Load configuration
def load_config(filename="config.json"):
    try:
        print(f"Loading configuration from {filename}...")
        with open(filename, "r") as file:
            config = json.load(file)
            print("✅ Configuration loaded successfully!")
            return config
    except (FileNotFoundError, json.JSONDecodeError) as e:
        print(f"❌ Error loading config: {e}")
        raise

config = load_config()

# Configuration details
MQTT_SERVER = config["mqtt_server"]
MQTT_PORT = config["mqtt_port"]
PUBLISH_TOPIC = "turnstile_publish"
SUBSCRIBE_TOPIC = "turnstile_subscribe"
PORT_ID = config["port_id"]

# Modbus Configuration
DEVICE_SLAVE_ADDRESS = config["modbus_slave_address"]
FUNCTION_CODE = 0x04  # Read Input Registers
COM_PORT_NAME = config["com_port_name"]
BAUD_RATE = config["baud_rate"]

# MQTT Client initialization
client = mqtt.Client()

# CRC16 Calculation (Modbus CRC)
def ModRTU_CRC(data):
    crc = 0xFFFF
    for pos in data:
        crc ^= pos
        for _ in range(8):
            if (crc & 0x0001) != 0:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF

# Modbus request and response handler
def read_register(register):
    print(f"🔍 Reading register {register} via Modbus...")
    try:
        ser = serial.Serial(COM_PORT_NAME, BAUD_RATE, 8, 'N', 1, timeout=3)
        if ser.is_open:
            print("✅ Serial port opened successfully!")
            # Prepare Modbus request
            req = [DEVICE_SLAVE_ADDRESS, FUNCTION_CODE, 0x00, register, 0x00, 0x02]
            crc = ModRTU_CRC(req)
            req.append(crc & 0xFF)  # Low byte of CRC
            req.append((crc >> 8) & 0xFF)  # High byte of CRC

            print(f"Sending Modbus request: {req}")
            ser.write(bytearray(req))
            response = ser.read(9)  # Expected response length
            print(f"Received response: {list(response)}")

            ser.close()
            print("✅ Serial port closed successfully!")

            # Process response
            if len(response) >= 7:  # Valid response
                data_bytes = response[3:7]
                float_value = struct.unpack('>f', data_bytes)[0]  # Big-endian float
                print(f"✅ Successfully read value: {float_value}")
                return "", float_value
            else:
                print("⚠️ Invalid response length or format.")
                return "FAILURE", 0.00
        else:
            print("❌ Serial port failed to open.")
            return "FAILURE", 0.00
    except Exception as e:
        print(f"❌ Error while reading Modbus register: {e}")
        return "FAILURE", 0.00

# MQTT Callback Functions
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ Connected to MQTT Broker at {MQTT_SERVER}:{MQTT_PORT}")
        client.subscribe(SUBSCRIBE_TOPIC)
        print(f"✅ Subscribed to topic: {SUBSCRIBE_TOPIC}")
    else:
        print(f"❌ Failed to connect to MQTT Broker. Return code: {rc}")

def on_message(client, userdata, msg):
    print(f"📩 Received MQTT message on topic {msg.topic}: {msg.payload.decode()}")
    try:
        payload = json.loads(msg.payload.decode())
        received_value = int(payload.get('message', 0))  # Default to 0 if 'message' is missing

        if 1 <= received_value <= 10:
            print(f"📤 Valid message received. Transmitting via Modbus: {received_value}")
            transmit_to_modbus(received_value)
        else:
            print("⚠️ Received invalid message value.")
    except (ValueError, json.JSONDecodeError) as e:
        print(f"❌ Error processing MQTT message: {e}")

# Transmit data via Modbus
def transmit_to_modbus(value):
    print(f"🔗 Transmitting value {value} via Modbus...")
    MODBUS_WRITE_REGISTER = config["modbus_write_register"]

    # Send data as a Modbus write request
    status, _ = read_register(MODBUS_WRITE_REGISTER)
    if status == "FAILURE":
        print(f"❌ Failed to transmit Modbus data: {value}")
    else:
        print(f"📤 Successfully transmitted data to register {MODBUS_WRITE_REGISTER}")

# Publish MQTT message
def publish_message(client, uart_id, command, turnstile_id):
    timestamp = get_timestamp()
    payload = {
        "RFID": uart_id,
        "entry": command,
        "turnstileID": turnstile_id,
        "timeStamp": timestamp,
        "portId": PORT_ID
    }
    print(f"Preparing to publish MQTT message: {payload}")
    try:
        result = client.publish(PUBLISH_TOPIC, json.dumps(payload))
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"✅ Published successfully: {payload}")
        else:
            print(f"❌ Failed to publish message. Return code: {result.rc}")
    except Exception as e:
        print(f"❌ Exception while publishing message: {e}")

# Generate timestamp
def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Improved MQTT reconnection logic
def mqtt_reconnect():
    while True:
        try:
            print("🔄 Attempting to reconnect to MQTT Broker...")
            client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
            print("✅ Reconnected to MQTT Broker.")
            client.loop_start()
            break
        except Exception as e:
            print(f"❌ Reconnection failed: {e}")
            time.sleep(5)

# Main function
def main():
    print("🚀 Starting program...")
    try:
        print(f"Connecting to MQTT Broker at {MQTT_SERVER}:{MQTT_PORT}...")
        client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
        client.loop_start()
        print("✅ MQTT loop started.")
    except Exception as e:
        print(f"❌ Initial connection failed: {e}")
        mqtt_reconnect()

    try:
        while True:
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("🛑 Shutting down gracefully...")
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    # Assign MQTT callbacks
    client.on_connect = on_connect
    client.on_message = on_message

    # Start the main loop
    main()