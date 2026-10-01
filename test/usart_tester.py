import serial
import time

# Set correct COM-port (Windows: 'COM3', 'COM4' etc. / Linux & Mac: '/dev/ttyUSB0' or '/dev/ttyACM0')
PORT = 'COM3' 
BAUDRATE = 115200

def send_command(ser, command):
# Tyhjennetään vanhat roskat pois puskurista ennen lähetystä
    ser.reset_input_buffer()
    
    # Lähetetään komento \r\n kanssa
    msg = (command.strip() + "\r\n").encode('utf-8')
    ser.write(msg)
    
    # Annetaan Arduinolle 100ms aikaa käsitellä ja vastata
    time.sleep(0.1)
    
    # Luetaan kaikki tavut jotka Arduinolta on tullut
    if ser.in_waiting > 0:
        response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore').strip()
        return response
    else:
        return None # Mitään ei tullut puskuriin!

def main():
    try:
        # Open serialport
        print(f"Yhdistetään porttiin {PORT}...")
        ser = serial.Serial(PORT, BAUDRATE, timeout=2)
        
        # Important: Arduino boots itself when serialport is opened.
        # Wait 2 seconds that Arduino has time to restart!
        time.sleep(2)
        
        # TEmpty possible carbage from buffer after startup
        ser.reset_input_buffer()
        print("Connection established to arduino board!\n")

        # Interactive loop for commands
        while True:
            cmd = input("Enter command (i.e. TEMP0) or 'exit': ").strip()
            
            if cmd.lower() == 'exit':
                print("Closing connection...")
                break
                
            if not cmd:
                continue

            # send command to arduino
            response = send_command(ser, cmd)
            
            if response:
                print(f"Response from arduino-> {response}\n")
            else:
                print("No response.... (timeout).\n")

    except serial.SerialException as e:
        print(f"Error in serial port connection: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serialport is closed.")

if __name__ == "__main__":
    main()