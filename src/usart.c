#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define RX_BUFFER_SIZE 64
static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

void usart_init(uint32_t baudrate) {
    // Claculate UBRR-register value with F_CPU and baudrate
    // Using formula: UBRR = (F_CPU / (16 * Baud)) - 1
	UCSR0A |= (1 << U2X0);

    uint16_t ubrr_value = (uint16_t)((F_CPU / (8UL * baudrate)) - 1);

    // Set baudrate UBRR0H ja UBRR0L
    UBRR0H = (uint8_t)(ubrr_value >> 8);
    UBRR0L = (uint8_t)(ubrr_value);

    // Switch on reception (RX) and transmit (TX)
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Set frame format: 8 data bits, 1 stop bit, no parity (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

}

void usart_transmit(char data) {
    //Wait until send buffer (UDRE0) is empty
    while (!(UCSR0A & (1 << UDRE0))) {}
    // Write data to buffer which starts the data sending
    UDR0 = data;
}

char usart_receive(void) {
    // Wait until daat is completly received (RXC0-flag is up)
    while (!(UCSR0A & (1 << RXC0))) {}
    // Return received data for further investigation
    return UDR0;
}

uint8_t usart_receive_string(char *buffer, uint8_t max_length) {
    uint8_t i = 0;
    char c;

	// Leave 1 byte tp null terminator '\0'
    while (i < (max_length - 1)) { 
		// Wait next character
        c = usart_receive(); 

		if (i == 0 && (c == '\r' || c == '\n')) {
            continue;
        }

        // If it is linefeed or carraige return (\r tai \n), stop reading
        if (c == '\r' || c == '\n') {
            break;
        }

        buffer[i] = c;
        i++;
    }

	// Append null termination according to C-standard
    buffer[i] = '\0'; 
    return i;
}

void usart_print(const char *str) {
    while (*str != '\0') {
        usart_transmit(*str);
        str++;
    }
}

uint8_t usart_available(void) {
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

void usart_init_slave_interrupt(uint32_t baudrate) {

	// 1. Synkronisessa tilassa UBRR-kaavan kerroin on 2UL (ei 16UL)
    uint16_t ubrr_value = (uint16_t)((F_CPU / (2UL * baudrate)) - 1);
    UBRR0H = (uint8_t)(ubrr_value >> 8);
    UBRR0L = (uint8_t)(ubrr_value);

    // 2. Aseta XCK-pinni (PB5 ATmega328P:ssä) SISÄÄNTULOKSI (Slave-tila)
    DDRB &= ~(1 << DDB5);

    // 3. Ota käyttöön RX, TX ja RX-keskeytys (RXCIE0)
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // 4. Aseta Synkroninen tila (UMSEL00 = 1, UMSEL01 = 0) ja 8 databittiä
    UCSR0C &= ~(1 << UMSEL01);
    UCSR0C |=  (1 << UMSEL00) | (1 << UCSZ01) | (1 << UCSZ00);

    // 5. Salli globaalit keskeytykset
    sei();
}

// Interrupt vector function. This is invoked when ever there is 
// new data available in RX buffer
ISR(USART_RX_vect) {
    char data = UDR0;
    uint8_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;

    // Jos puskuri ei ole täynnä, tallennetaan tavu
    if (next_head != rx_tail) {
        rx_buffer[rx_head] = data;
        rx_head = next_head;
    }
}

// Luetaan yksi tavu rengaspuskurista (ei lukitse suoritusta)
int16_t usart_read_byte_interrupt(void) {
    if (rx_head == rx_tail) {
        return -1; // Puskuri tyhjä
    }

    char data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return data;
}