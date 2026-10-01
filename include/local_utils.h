#ifndef LOCAL_UTILS_H
#define LOCAL_UTILS_H

#include <avr/eeprom.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t load_eeprom_word(const uint16_t *eeprom_addr) {
    return eeprom_read_word(eeprom_addr);
}

static inline void save_eeprom_word(uint16_t *eeprom_addr, uint16_t value) {
    eeprom_update_word(eeprom_addr, value);
}

#ifdef __cplusplus
}
#endif

#endif