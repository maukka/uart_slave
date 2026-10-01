#ifndef USART_H
#define USART_H

#include <stdint.h>

// Initializes USART-bus to given baudrate (Uses F_CPU-constant)
void usart_init(uint32_t baudrate);

// Sends one char over USART
void usart_transmit(char data);

// Waits one char and reads it from data register
char usart_receive(void);
uint8_t usart_receive_string(char *buffer, uint8_t max_length);
int16_t usart_read_byte_intrerrupt(void);
// Sends char array (ends to null-char '\0')
void usart_print(const char *str);

// Checks if there is more data available (1 = yes, 0 = no)
uint8_t usart_available(void);


#endif // USART_H