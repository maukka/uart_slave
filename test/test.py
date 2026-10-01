import serial
import time

PORT = 'COM3'
BAUD = 115200

print(f"Opening {PORT} ({BAUD} baud)...")
ser = serial.Serial(PORT, BAUD, timeout=2)

# Wait DTR reset 2.5s / over bootload
print("Wait chip boots...")
time.sleep(2.5)

# Clear the buffer
ser.reset_input_buffer()

commands = ["TEMP0", "LED_ON", "LED_OFF", "CALIBRATE 21", "TEST_ERR"]

for cmd in commands:
    print(f"\n---> Sending command: {cmd}")
    ser.write((cmd + "\r\n").encode('utf-8'))
    
    # Little wait delay for chip
    time.sleep(0.15)
    
    if ser.in_waiting > 0:
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        print(f"<--- Response: {response}")
    else:
        print("<--- No response (Timeout/In_waiting = 0)")

ser.close()