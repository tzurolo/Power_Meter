//
//  I2C Asynchronous interface - Master only
//
//  What it does:
//    Interface to AVR hardware I2C. The functions in this unit return
//    immediately.
//    Handler functions are called when the read or write functions complete.
//    Based on AtMega32U4 spec
//
//  How to use it:
//    Call I2CAsync_Initialize() once at the beginning of the program (powerup)
//    Call I2CAsync_task() every iteration of the program mainloop.
//    Optionally define a completion handler to be called when the I2C transfer
//    completes.
//    If I2CAsync_isIdle() returns false you can call I2CAsync_transferData()
//    to write data and the completion handler 
//

#ifndef I2CASYNC_H
#define I2CASYNC_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

// Status codes from I2C TWSR
typedef enum I2CStatusCode_enum {
    isc_startTransmitted = 0x08,
    isc_repeatedStartTransmitted = 0x10,
    isc_SLAWACK = 0x18,
    isc_SLAWNACK = 0x20,
    isc_dataTransmittedAck = 0x28,
    isc_dataTransmittedNack = 0x30,
    isc_SLAArbitrationLost = 0x38,
    isc_SLARACK = 0x40,
    isc_SLARNACK = 0x48,
    isc_dataReceivedAck = 0x50,
    isc_dataReceicedNack = 0x58,
    isc_timeout = 0x7F
} I2CStatusCode;

typedef void (*I2CAsync_CompletionHandler)(
    const bool success,
    const I2CStatusCode i2cStatus,
    const uint8_t readDataLength,
    const uint8_t* readData);

extern void I2CAsync_Initialize (void);

extern void I2CAsync_task (void);

extern bool I2CAsync_isIdle (void);

// sends the given data bytes to the given address.
// calls writeHandler when the operation is complete.
// returns false if there is a read or write already
// in progress.
extern bool I2CAsync_transferData (
    const uint8_t address,
    const uint8_t writeDataLength,
    const uint8_t *writeData,
    const uint8_t readDataLength,
    uint8_t *readData,
    I2CAsync_CompletionHandler completionHandler);

#endif      // I2CASYNC_H
