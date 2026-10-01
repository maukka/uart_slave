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

int main(void) {
    char cmd_buffer[32];

    // Sisäänrakennettu L-ledi lähtötilaksi
    DDRB |= (1 << PB5);

    // Alustetaan USART 115200 baud
    usart_init(115200);
	init_temperature_sensor();

    // Lähetetään käynnistysviesti sarjaporttiin
    usart_print("SYSTEM: READY\r\n");

    while (1) {
        // Luetaan komento VAIN jos sarjaportissa on dataa odottamassa (ei lukitse suoritusta)
        if (usart_available()) {
            
            // Luetaan koko merkkijono rivinvaihtoon asti
            usart_receive_string(cmd_buffer, sizeof(cmd_buffer));

            // Käsitellään komennot
            if (strcmp(cmd_buffer, "TEMP0") == 0) {
                // Testivastaus kovakoodatulla arvolla sarjaliikenteen varmistamiseksi
				uint16_t result = read_temp_celsius();
				char response[64];
				sprintf(response, "Temperature in board arduino %d C\r\n", result);
                usart_print(response);
				//usart_print("RESPONSE: OK (TEMP0)\r\n");
            } 
            else if (strcmp(cmd_buffer, "LED_ON") == 0) {
                PORTB |= (1 << PB5);
                usart_print("RESPONSE: OK (LED ON)\r\n");
            } 
            else if (strcmp(cmd_buffer, "LED_OFF") == 0) {
                PORTB &= ~(1 << PB5);
                usart_print("RESPONSE: OK (LED OFF)\r\n");
            }
			// Käsitellään komento "CALIBRATE <lämpötila>"
			else if (strncmp(cmd_buffer, "CALIBRATE ", 10) == 0) {
				// Luetaan asteluku tekstin perästä (esim. "CALIBRATE 23")
				int16_t target_temp = (int16_t)atoi(&cmd_buffer[10]);

				if (calibrate_temperature_sensor(target_temp)) {
					char response[64];
					uint16_t new_offset = load_eeprom_word(&eeprom_temperature_offset);
					sprintf(response, "RESPONSE: OK (Calibrated to %d C, New Offset: %u)\r\n", 
							target_temp, new_offset);
					usart_print(response);
				} else {
					usart_print("ERROR: Calibration failed (Invalid temp or offset out of bounds)\r\n");
				}
			} 
            else if (strlen(cmd_buffer) > 0) {
                // Tulostetaan tuntematon komento diagnosointia varten
                char err_msg[64];
                sprintf(err_msg, "ERROR: Unknown Command ('%s')\r\n", cmd_buffer);
                usart_print(err_msg);
            }
        }

        // Ledi vilkkuu ilmaisten että main-silmukka pyörii
        //PORTB ^= (1 << PB5);
        //_delay_ms(100); 
    }
}