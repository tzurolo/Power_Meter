//
// EEPROM access
//
#ifndef EEPROM_H
#define EEPROM_H

#include <avr/io.h> // for integer type definitions

extern void EEPROM_write (
    const unsigned int uiAddress,
    const uint8_t ucData);

uint8_t EEPROM_read (
    const unsigned int uiAddress);

#endif  // EEPROM_H
