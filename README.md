# ATmega328P Temperature Sensor & Calibration Controller

Powefull, non-blocking bare-metal C-implementation for ATmega328P-MCU (16 MHz). Project uses buildin temperature sensor on channel ADC8, filters noice by averanging temperature samples, controls calibrated values in  EEPROM-memory and offers two-way serial communication channel (115200 baud) to control the board. Supports also slave mode by setting -DUSE_SLAVE_MODE=1 in Makefile. In slave
mode you can safely connect ATMega328P to another chip (acting as master in USART communication) for exchanging data between chips.

## 🚀 Features

* **Filtered temperature measurement:** build in ADC8-channel 16/32 sample averanging from 1.1V internal reference.
* **Static EEPROM-calibrartion:** Automatic offset-calculation and storing directly via USART (`CALIBRATE <temp>`).
* **High performance USART:** Buffered serial transmitting 115200 baud -speed without delays (`_delay_ms`). Using pins PD5 (TX) and PD4 (RX).
* **Modular C-architecture:** `static inline` -utility functions for EEPROM memory and `extern "C"` -compatibility for compiler.
* **Automatic testing:** Python script included for automatic testing and calibrartion.
* **Slave mode:** Supports synchronous slave mode by setting -DUSE_SLAVE_MODE=1 in Makefile.

## 🛠️ Hardware requirements and tools

* **MCU:** ATmega328P (e.g. Arduino Uno / Nano)
* **Clock frequency:** 16 MHz
* **Compiler & Tools:** `avr-gcc`, `avr-libc`, `avrdude`, `make`
* **Testing:** Python 3.x and `pyserial`-kirjasto

## 📁 Project hierarchy

```text
.
├── include/
│   ├── my_utils.h      # Reading and writing EEPROM-memory
│   ├── temp_sensor.h   # Temperature sensor API
│   └── usart.h         # USART-serial communication 
├── src/
│   ├── main.c          # Main loop and command handling
│   ├── temp_sensor.c   # ADC8-measurement and EEPROM based calibration
│   └── usart.c         # Buffered USART implementation
├── test.py             # Automatic Python based test scripts.
├── Makefile            # Compile & linking rules, contains also testing and flashing functionality
└── README.md