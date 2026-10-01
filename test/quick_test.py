import serial, time

ser = serial.Serial('COM3', 115200, timeout=2)
time.sleep(2.5) # Odotetaan DTR-resettiä

ser.reset_input_buffer()
print("Lähetetään komento...")
ser.write(b"TEMP0\r\n")

time.sleep(0.5)
print(f"Puskurissa odottavien tavujen määrä: {ser.in_waiting}")
print(f"Raakasisältö: {ser.read_all()}")
ser.close()