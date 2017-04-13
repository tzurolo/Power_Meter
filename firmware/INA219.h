//
// INA219 high side current sensor
//  on Adafruit breakout
//
//  How to use it
//  After calling one of the functions you must wait until its
//  completion handler is called before calling another function
//
#ifndef INA219_H
#define INA219_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "I2CAsync.h"

// Bus voltage range
typedef enum INA219BRNG_enum {
    ibrng_16V = 0,
    ibrng_32V = 1
} INA219BRNG;

// PGA gains
typedef enum INA219PGA_enum {
    ipga_unity = 0,
    ipga_div2  = 1,
    ipga_div4  = 2,
    ipga_div8  = 3
} INA219PGA;

// ADC resolution / averaging
typedef enum INA219ADC_enum {
    iadc_9bit      = 0,
    iadc_10bit     = 1,
    iadc_11bit     = 2,
    iadc_12bit     = 3,
    iadc_1sample   = 8,
    iadc_2sample   = 9,
    iadc_4sample   = 10,
    iadc_8sample   = 11,
    iadc_16sample  = 12,
    iadc_32sample  = 13,
    iadc_64sample  = 14,
    iadc_128sample = 15
} INA219ADC;

// mode settings
typedef enum INA219Mode_enum {
    im_powerDown             = 0,
    im_shuntTriggered        = 1,
    im_busTriggered          = 2,
    im_shuntAndBusTriggered  = 3,
    im_ADCOff                = 4,
    im_shuntContinuous       = 5,
    im_busContinuous         = 6,
    im_shuntAndBusContinuous = 7
} INA219Mode;

// register pointers
typedef enum INA219RegisterAddr_enum {
    ira_configuration    = 0,
    ira_shuntVoltage     = 1,
    ira_busVoltage       = 2,
    ira_powerMeasurement = 3,
    ira_current          = 4,
    ira_calibration      = 5
} INA219RegisterAddr;

typedef void (*INA219_WriteCompletionHandler)(
    const bool success,
    const I2CStatusCode i2cStatus);
typedef void (*INA219_ReadCompletionHandler)(
    const bool success,
    const I2CStatusCode i2cStatus,
    const int16_t registerValue);

// returns true if the I2C interface is available
extern bool INA219_setConfiguration (
    const bool reset,
    const INA219BRNG busVoltageRange,
    const INA219PGA pga,
    const INA219ADC busAdc,
    const INA219ADC shuntAdc,
    const INA219Mode mode,
    INA219_WriteCompletionHandler completionHandler);

// returns true if the I2C interface is available
extern bool INA219_setCalibration (
    const uint16_t fullScaleDrop,
    INA219_WriteCompletionHandler completionHandler);

// sets the register pointer address to the register intended to be
// read by INA219_readRegister
// returns true if the I2C interface is available
extern bool INA219_setRegisterPtr (
    const INA219RegisterAddr registerAddr,
    INA219_WriteCompletionHandler completionHandler);

// reads the register that the current register pointer points to
// returns true if the I2C interface is available
extern bool INA219_readRegister (
    INA219_ReadCompletionHandler completionHandler);

#endif  // INA219_H