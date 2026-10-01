#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "../include/local_utils.h"
#include "../include/temp_sensor.h"

#define ADC_GAIN_X1000  1061    // 1.061 * 1000 (asteen muutos ADC-askelina)
#define DEFAULT_OFFSET  301     // Tehdas-oletus 0 °C raaka-arvolle

uint16_t EEMEM eeprom_temperature_offset;

void init_temperature_sensor(void){

	PRR &= ~(1 << PRADC);
	// Select channel ADC8 this is where temperatuer sensor
	// is routed.
	ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3);

	// Enable ADC  
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
	// ADC Enable
	ADCSRB = 0;
	// There is no value stored for calibrated temperature
	// use default offset
	if(load_eeprom_word(&eeprom_temperature_offset) == 0xFFFF){
		save_eeprom_word(&eeprom_temperature_offset, DEFAULT_OFFSET);
	}

	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
}

uint16_t read_temp_raw(void) {
    // Käynnistetään ADC-muunnos (ADSC)
    ADCSRA |= (1 << ADSC);

    // Odotetaan kunnes muunnos on valmis (ADSC-bitti nollautuu)
    while (ADCSRA & (1 << ADSC));

    // Palautetaan 10-bittinen ADC-raaka-arvo (ADCW / ADC)
    return ADC;
}

int16_t read_temp_celsius(void) {
    uint16_t raw = read_temp_raw();
    
	// Säädä offset-arvoa (esim. 315..325) sen mukaan, mikä vastaa huoneesi lämpötilaa.
    // Kerrotaan 100 ja jaetaan 122, jotta vältetään raskas float-laskenta:
    int16_t temp_c = (int16_t)(((int32_t)(raw - 315) * 100) / 122);
    return temp_c;
}

uint16_t read_temp_raw_averaged(uint8_t samples) {
    uint32_t sum = 0;

    for (uint8_t i = 0; i < samples; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        sum += ADC;
    }

    return (uint16_t)(sum / samples);
}

int16_t read_temp_celsius_calibrated(void) {
    // 1. Otetaan 16 näytteen keskiarvo kohinan poistamiseksi
    uint16_t raw_avg = read_temp_raw_averaged(16);

    // 2. Lasketaan ero 0 °C vertailupisteeseen
    int32_t adc_diff = (int32_t)raw_avg - eeprom_temperature_offset;

    // 3. Muunnetaan Celsius-asteiksi kiintolukulaskennalla: (diff * 1000) / 1061
    int16_t temp_c = (int16_t)((adc_diff * 1000) / ADC_GAIN_X1000);

    return temp_c;
}

bool calibrate_temperature_sensor(int16_t actual_temp_c) {
    // Turvaraja syötettävälle lämpötilalle (-20 ... +60 °C)
    if (actual_temp_c < -20 || actual_temp_c > 60) {
        return false;
    }

    // 1. Otetaan kohinaton raaka-arvo (32 näytteen keskiarvo)
    uint16_t raw_avg = read_temp_raw_averaged(32);

    // 2. Lasketaan kuinka monta ADC-askelta nykyinen lämpötila vastaa 0 °C nollakohdasta
    int32_t temp_adc_delta = ((int32_t)actual_temp_c * ADC_GAIN_X1000) / 1000;

    // 3. Lasketaan uusi 0 °C offset-arvo
    int32_t new_offset = (int32_t)raw_avg - temp_adc_delta;

    // Varmistetaan, että laskettu offset on järkevä ATmega328P:lle (~250...380)
    if (new_offset >= 250 && new_offset <= 380) {
        save_eeprom_word(&eeprom_temperature_offset, (uint16_t)new_offset);
        return true;
    }

    return false;
}