import serial
import time
import paho.mqtt.client as mqtt
import json
from datetime import datetime

# Load configuration
def load_config(filename="config.json"):
    try:
        with open(filename, "r") as file:
            return json.load(file)
    except Exception as e:
        print(f"❌ Error loading config: {e}")
        raise

config = load_config()

# Configuration
MQTT_SERVER = config["mqtt_server"]
MQTT_PORT = config["mqtt_port"]
PUBLISH_TOPIC = "turnstile_publish"
SUBSCRIBE_TOPIC = "turnstile_subscribe"
COM_PORT_NAME = config["com_port_name"]
BAUD_RATE = config["baud_rate"]
SERIAL_TIMEOUT = 0.1  # Timeout for serial reads
DATA_SIZE = 14  # Total bytes to read for RFID data
port_Id = "P02"

# Initialize MQTT client
client = mqtt.Client()

# Initialize serial port
serial_port = serial.Serial(COM_PORT_NAME, BAUD_RATE, timeout=SERIAL_TIMEOUT)

# Global flag to hold MQTT received data
mqtt_response = None


# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ Connected to MQTT Broker at {MQTT_SERVER}:{MQTT_PORT}")
        client.subscribe(SUBSCRIBE_TOPIC)
    else:
        print(f"❌ MQTT connection failed with code {rc}")

def on_message(client, userdata, msg):
    global mqtt_response
    print(f"📩 MQTT Message: {msg.payload.decode()}")
    try:
        payload_str = msg.payload.decode("utf-8")
        print("message decoded: ", payload_str)

        payload = int(payload_str)
        print("received integer payload: ", payload)
        
        #mqtt_response = payload
        mqtt_response = payload_str
        #mqtt_response = payload.to_bytes(2, byteorder="big", signed=False)
        print("converted to bytes: ", mqtt_response)
    except ValueError as e:
        print("Payload isnt valid integer")
    except json.JSONDecodeError as e:
        print(f"❌ Error decoding MQTT message: {e}")


client.on_connect = on_connect
client.on_message = on_message

# Send Serial Data
def send_serial(data):
    """Send data via UART."""
    if serial_port.is_open:
        #data = data.encode('utf-8')
        serial_port.write(data.encode('utf-8'))
        print(f"📤 Sent to UART: {data.encode('utf-8')}")

# Read Serial Data
def read_serial():
    """Read and validate data from UART."""
    try:
        if serial_port.is_open:
            print("Serial Port Open")
            data = serial_port.read(DATA_SIZE)
            print("Reading Data:", data)  # Read exactly 14 bytes
            if len(data) == DATA_SIZE:
                print(f"📥 Received from UART: {data}")
                rfid = data[:12].decode('utf-8')  # First 12 bytes are RFID
                entry_exit = data[12]  # 13th byte
                turnstile_id = data[13]  # 14th byte
                return rfid, entry_exit, turnstile_id
            else:
                print(f"⚠️ Incomplete data received: {data}")
    except Exception as e:
        print(f"❌ Serial error: {e}")
    return None, None, None

# Publish MQTT Message
def publish_message(client, rfid, entry_exit, turnstile_id):
    """Encapsulate data in JSON and publish via MQTT."""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    payload = {
        "RFID": rfid,
        "entry": entry_exit,
        "turnstileID": turnstile_id,
        "timeStamp": timestamp,
        "portId": port_Id
    }
    print(f"📤 Publishing MQTT message: {payload}")
    result = client.publish(PUBLISH_TOPIC, json.dumps(payload))
    if result.rc == mqtt.MQTT_ERR_SUCCESS:
        print(f"✅ Published successfully: {payload}")
    else:
        print(f"❌ Failed to publish MQTT message.")

# Main loop
def main():
    global mqtt_response
    try:
        # Connect to MQTT broker
        client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
        client.loop_start()
        print("🚀 MQTT Loop started.")

        while True:
            # Step 1: Read from Serial
            rfid, entry_exit, turnstile_id = read_serial()
            if rfid and entry_exit is not None and turnstile_id is not None:
                # Step 2: Publish to MQTT
                publish_message(client, rfid, entry_exit, turnstile_id)

                # Step 3: Wait for MQTT Response
                mqtt_response = None
                timeout = 10  # Wait up to 5 seconds for MQTT response
                start_time = time.time()
                while mqtt_response is None and time.time() - start_time < timeout:
                    time.sleep(0.1)

                if mqtt_response:
                    # Step 4: Send response back via Seria
                    send_serial(mqtt_response)
                    print("Transmitting UART: ", mqtt_response)
                else:
                    print("⚠️ No MQTT response received within timeout.")
                    send_serial("21")
                    print("Sending error code 21")
            time.sleep(0.5)

    except KeyboardInterrupt:
        print("🛑 Program interrupted.")
    finally:
        # Clean up
        client.loop_stop()
        client.disconnect()
        if serial_port.is_open:
            serial_port.close()
        print("🔌 Serial port closed.")

if __name__ == "__main__":
    main()