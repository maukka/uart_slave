#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/usart.h"
#include "../include/temp_sensor.h"
#include "../include/local_utils.h"

int process_command(const char *cmd, char *response, size_t response_size) {
	if (strcmp(cmd, "TEMP0") == 0) {
		uint16_t result = read_temp_celsius();
		snprintf(response, response_size, "Temperature in board arduino %d C\r\n", result);
		usart_print(response);
		return 1;
	} 
	else if (strcmp(cmd, "LED_ON") == 0) {
		PORTB |= (1 << PB5);
		snprintf(response, response_size, "RESPONSE: OK (LED ON)\r\n");
		usart_print(response);
		return 1;
	} 
	else if (strcmp(cmd, "LED_OFF") == 0) {
		PORTB &= ~(1 << PB5);
		snprintf(response, response_size, "RESPONSE: OK (LED OFF)\r\n");
		usart_print(response);
		return 1;
	}
	else if (strncmp(cmd, "CALIBRATE ", 10) == 0) {
		int16_t target_temp = (int16_t)atoi(&cmd[10]);
		if (calibrate_temperature_sensor(target_temp)) {
			uint16_t new_offset = load_eeprom_word(&eeprom_temperature_offset);
			snprintf(response, response_size, "RESPONSE: OK (Calibrated to %d C, New Offset: %u)\r\n", 
					target_temp, new_offset);
			usart_print(response);
			return 1;
		} else {
			snprintf(response, response_size, "ERROR: Calibration failed (Invalid temp or offset out of bounds)\r\n");
			usart_print(response);
			return 0;
		}
	} 
	else {
		snprintf(response, response_size, "ERROR: Unknown Command ('%s')\r\n", cmd);
		usart_print(response);
		return 0;
	}
}

int main(void) {
    char cmd_buffer[32];
	uint8_t cmd_idx = 0;
    char response_buffer[64];

	DDRB |= (1 << DDB5);

#if USE_SLAVE_MODE
    usart_init_slave_interrupt(115200);
#else
    usart_init(115200);
#endif
	init_temperature_sensor();

    // Lähetetään käynnistysviesti sarjaporttiin
    usart_print("SYSTEM: READY\r\n");

    while (1) {
		// Read byte from RX buffer
        int16_t b = usart_read_byte_interrupt();

        if (b != -1) {
            char c = (char)b;

            // If charater ise linefeed or carriage return we have read the whole command
            if (c == '\n' || c == '\r') {
                if (cmd_idx > 0) {
                    cmd_buffer[cmd_idx] = '\0'; 
                    
                    // Call process command which will handle the command and send response
                    process_command(cmd_buffer, response_buffer, sizeof(response_buffer));
                    
                    cmd_idx = 0;
                }
            } 
            // Not an linefeed of carriage return, so we can add it to command buffer if there is space
            else if (cmd_idx < (sizeof(cmd_buffer) - 1)) {
                cmd_buffer[cmd_idx++] = c;
            }
        }
    }
}