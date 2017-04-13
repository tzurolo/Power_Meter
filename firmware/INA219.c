//
// INA219 high side current sensor
//  on Adafruit breakout
//

#include "INA219.h"

#include "CharString.h"
#include "StringUtils.h"
#include "Console.h"

#define INA219_I2C_ADDR 0x40

#define PTR_LEN 1
#define REGISTER_DATA_LEN 2
#define CONFIGURATION_DATA_LEN 3
#define DATA_BUFFER_LEN 3

static INA219_WriteCompletionHandler clientWriteCompletionHandler = 0;
static INA219_ReadCompletionHandler clientReadCompletionHandler = 0;
static uint8_t dataBuffer[DATA_BUFFER_LEN];

void writeHandler (
    const bool success,
    const I2CStatusCode i2cStatus,
    const uint8_t readDataLength,
    const uint8_t* readData)
{
    if (clientWriteCompletionHandler != 0) {
        clientWriteCompletionHandler(success, i2cStatus);
    }
}

void readHandler (
    const bool success,
    const I2CStatusCode i2cStatus,
    const uint8_t readDataLength,
    const uint8_t* readData)
{
    if (clientReadCompletionHandler != 0) {
        // assuming readDataLength == 2
        const int16_t registerValue = (readData[0] << 8) + readData[1];
        clientReadCompletionHandler(success, i2cStatus, registerValue);
    }
}

bool INA219_setConfiguration (
    const bool reset,
    const INA219BRNG busVoltageRange,
    const INA219PGA pga,
    const INA219ADC busAdc,
    const INA219ADC shuntAdc,
    const INA219Mode mode,
    INA219_WriteCompletionHandler completionHandler)
{
    dataBuffer[0] = ira_configuration;
    const int16_t configurationWord = 
        ((reset ? 1 : 0) << 15) |
        (busVoltageRange << 13) |
        (pga << 11) |
        (busAdc << 7) |
        (shuntAdc << 3) |
        mode;

    CharString_define(60, msg);
    CharString_copyP(PSTR("config: "), &msg);
    StringUtils_appendDecimal(configurationWord, 1, 0, &msg);
    Console_printCS(&msg);

    dataBuffer[1] = (configurationWord >> 8) & 0xFF;
    dataBuffer[2] = configurationWord & 0xFF;
    clientWriteCompletionHandler = completionHandler;
    return I2CAsync_transferData(INA219_I2C_ADDR,
        CONFIGURATION_DATA_LEN, dataBuffer,
        0, 0,
        writeHandler);
}

bool INA219_setCalibration (
    const uint16_t fullScaleDrop,
    INA219_WriteCompletionHandler completionHandler)
{
    // not implemented yet
    return false;
}

bool INA219_setRegisterPtr (
    const INA219RegisterAddr registerAddr,
    INA219_WriteCompletionHandler completionHandler)
{
    dataBuffer[0] = registerAddr;
    clientWriteCompletionHandler = completionHandler;
    return I2CAsync_transferData(INA219_I2C_ADDR,
        PTR_LEN, dataBuffer,
        0, 0,
        writeHandler);
}

bool INA219_readRegister (
    INA219_ReadCompletionHandler completionHandler)
{
    clientReadCompletionHandler = completionHandler;
    return I2CAsync_transferData(INA219_I2C_ADDR,
        0, 0,
        REGISTER_DATA_LEN, dataBuffer,
        readHandler);
}
