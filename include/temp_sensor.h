#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H
#include <stdint.h>
#include <stdbool.h>
#include <avr/eeprom.h>

// Julistetaan EEPROM-muuttuja extern-määreellä muille tiedostoille
extern uint16_t EEMEM eeprom_temperature_offset;

void init_temperature_sensor(void);
uint16_t read_temp_raw(void);
int16_t read_temp_celsius(void);
uint16_t read_temp_raw_averaged(uint8_t samples);
bool calibrate_temperature_sensor(int16_t actual_temp_c);

#endif