import serial
import time

print("Avataan sarjaportti COM3...")
ser = serial.Serial('COM3', 115200, timeout=2)

print("Odotetaan Arduinon boottausta (3s)...")
time.sleep(3)

# Luetaan mitä Arduino lähetti käynnistyessään ("HELLO FROM ARDUINO")
boot_msg = ser.read_all().decode('utf-8', errors='ignore')
print(f"1. Boot-tervehdys Arduinolta: '{boot_msg.strip()}'")

print("Lähetetään kaikutesti 'X'...")
ser.write(b"X")
time.sleep(0.2)

echo_msg = ser.read_all().decode('utf-8', errors='ignore')
print(f"2. Kaikuvastaus Arduinolta: '{echo_msg}'")

ser.close()