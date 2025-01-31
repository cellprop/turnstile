import os

import sys

import serial

import time





#Ensure the script is run with sudo

if os.geteuid() != 0:

     os.execvp('sudo', ['sudo', '-E', sys.executable] + sys.argv)



# Configuration parameters

port = '/dev/ttyUSB0'  # Adjust this based on your device

baud_rate = 9600

buffer_size = 50  # Size of the buffer to store received data



# Set up the serial connection

serial_port = serial.Serial(port, baud_rate, timeout=0.1)





def send_message(data):

        if serial_port.is_open:

            serial_port.write(data.encode('utf-8'))

            print(f"Sent UART: {data}")







def read_from_port(ser):

    global buffer

    try:

          if serial_port.is_open:

                    print('serial port open')

                    data = ser.read(14)

                    print('reading data',data)

                    if data:

                     #buffer.extend(data)

                     #process_buffer()

                     valuedata = data.decode('utf-8')

                     print('data is ',valuedata)

                     #print("sending hello")

                     #send_message("hello")

    except (serial.SerialException, OSError) as e:

        print(f"Serial exception occurred: {e}")

        time.sleep(0.1)













print("Serial sending data. Press Ctrl+C to stop.")

try:

        

        time.sleep(1)



        while True:

           #read_from_port(serial_port) 

           send_message("hello!")

           time.sleep(1)

        

except KeyboardInterrupt:

    print("Program interrupted by user.")

finally:

    print("Serial port closed.")

