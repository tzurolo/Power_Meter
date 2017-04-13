//
//  I2C Asynchronous interface
//

#include "I2CAsync.h"
#include <avr/io.h>

#include "Console.h"
#include "SystemTime.h"
#include <stdlib.h>

#define TIMEOUT_INTERVAL (0.1 * SYSTEMTIME_TICKS_PER_SECOND)

typedef enum I2CState_enum {
    is_idle,
    is_waitingForWriteStartTransmission,
    is_waitingForSLAWTransmission,
    is_waitingForDataByteWrite,
    is_waitingForReadStartTransmission,
    is_waitingForSLARTransmission,
    is_waitingForDataByteRead
} I2CState;

// state variables
static I2CState i2cState;
static uint8_t i2cAddress;
static uint8_t i2cWriteDataLength;
static const uint8_t* i2cWriteData;
static uint8_t i2cReadDataLength;
static uint8_t* i2cReadData;
static uint8_t i2cDataCount;
static I2CAsync_CompletionHandler i2cCompletionHandler;
static SystemTime_t timeoutTime;

static void sendStart (void)
{
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
}

static void sendStop (void)
{
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

static I2CStatusCode i2cStatus (void)
{
    return ((I2CStatusCode)(TWSR & 0xF8));
}

static void terminateRead (
    const I2CStatusCode status)
{
    sendStop();
    i2cState = is_idle;
    i2cCompletionHandler(false, status, 0, NULL);
}

static void terminateWrite (
    const I2CStatusCode status)
{
    sendStop();
    i2cState = is_idle;
    i2cCompletionHandler(false, status, 0, NULL);
}

static void checkTimeout (void)
{
    if (SystemTime_timeHasArrived(&timeoutTime)) {
        sendStop();
        TWCR &= ~(1<<TWEN);
        i2cState = is_idle;
        i2cCompletionHandler(false, isc_timeout, 0, NULL);
    }
}

static void printStatus (void)
{
    const I2CStatusCode status = i2cStatus();
    CharString_define(20, str);
    CharString_copyP(PSTR("i2c status: "), &str);
    char statbuf[10];
    itoa((uint8_t)status, statbuf, 16);
    CharString_append(statbuf, &str);
    Console_printCS(&str);
}

void I2CAsync_Initialize (void)
{
    TWBR = 3;   // with a 16MHz CPU clock this makes the SCL frequency 400KHz

    i2cState = is_idle;
}

void I2CAsync_task (void)
{
    switch (i2cState) {
        case is_idle :
            break;
        case is_waitingForWriteStartTransmission :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if (status == isc_startTransmitted) {
                    // start has been transmitted
                    // send SLA+W
                    TWDR = (i2cAddress << 1) & 0xFE; 
                    TWCR = (1<<TWINT) | (1<<TWEN);
                    i2cState = is_waitingForSLAWTransmission;
                    SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                } else {
                    // unexpected status
                    terminateWrite(status);
                }
            } else {
                checkTimeout();
            }
            break;
        case is_waitingForSLAWTransmission :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if (status == isc_SLAWACK) {
                    // write first data byte
                    TWDR = i2cWriteData[0];
                    TWCR = (1<<TWINT) | (1<<TWEN);
                    i2cDataCount = 1;
                    i2cState = is_waitingForDataByteWrite;
                    SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                } else {
                    // unexpected status
                    terminateWrite(status);
                }
            } else {
                checkTimeout();
            }
            break;
        case is_waitingForDataByteWrite :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if (status == isc_dataTransmittedAck) {
                    if (i2cDataCount < i2cWriteDataLength) {
                        TWDR = i2cWriteData[i2cDataCount++];
                        TWCR = (1<<TWINT) | (1<<TWEN);
                    } else {
                        // all bytes written
                        sendStop();
                        if (i2cReadDataLength > 0) {
                            // we are reading data after this write
                            sendStart();
                            i2cState = is_waitingForReadStartTransmission;
                        } else {
                            i2cState = is_idle;
                            i2cCompletionHandler(true, status, 0, i2cReadData);
                        }
                    }
                    SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                } else {
                    // unexpected status
                    terminateWrite(status);
                }
            } else {
                checkTimeout();
            }
            break;
        case is_waitingForReadStartTransmission :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if (status == isc_startTransmitted) {
                    // start has been transmitted
                    // send SLA+R
                    TWDR = ((i2cAddress << 1) & 0xFE) | 0x01; 
                    TWCR = (1<<TWINT) | (1<<TWEN);
                    i2cState = is_waitingForSLARTransmission;
                    SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                } else {
                    // unexpected status
                    terminateRead(status);
                }
            } else {
                checkTimeout();
            }
            break;
        case is_waitingForSLARTransmission :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if (status == isc_SLARACK) {
                    // request first data byte
                    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
                    i2cDataCount = 0;
                    i2cState = is_waitingForDataByteRead;
                    SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                } else {
                    // unexpected status
                    terminateRead(status);
                }
            } else {
                checkTimeout();
            }
            break;
        case is_waitingForDataByteRead :
            if (TWCR & (1<<TWINT)) {
                const I2CStatusCode status = i2cStatus();
                if ((status == isc_dataReceivedAck) ||
                    (status == isc_dataReceicedNack)) {
                    i2cReadData[i2cDataCount++] = TWDR;
                    if (i2cDataCount < i2cReadDataLength) {
                        // request next or last data byte
                        TWCR = (1<<TWINT) | (1<<TWEN) | 
                            ((i2cDataCount < (i2cReadDataLength - 1))
                            ? (1<<TWEA) // next byte
                            : 0);       // last byte
                        SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);
                    } else {
                        // all bytes read
                        sendStop();
                        i2cState = is_idle;
                        i2cCompletionHandler(true, status, i2cDataCount, i2cReadData);
                    }
                } else {
                    // unexpected status
                    terminateRead(status);
                }
            } else {
                checkTimeout();
            }
            break;
    }
}

bool I2CAsync_isIdle (void)
{
    return (i2cState == is_idle);
}

extern bool I2CAsync_transferData (
    const uint8_t address,
    const uint8_t writeDataLength,
    const uint8_t *writeData,
    const uint8_t readDataLength,
    uint8_t *readData,
    I2CAsync_CompletionHandler completionHandler)
{
    bool startedSuccessfully = false;

    if (I2CAsync_isIdle()) {
        i2cAddress = address;
        i2cWriteDataLength = writeDataLength;
        i2cWriteData = writeData;
        i2cReadDataLength = readDataLength;
        i2cReadData = readData;
        i2cCompletionHandler = completionHandler;
        sendStart();
        i2cState = (writeDataLength > 0) 
            ? is_waitingForWriteStartTransmission
            : is_waitingForReadStartTransmission;
        SystemTime_futureTime(TIMEOUT_INTERVAL, &timeoutTime);

        startedSuccessfully = true;
    }

    return startedSuccessfully;
}
