//
// Power Meter
//
// uses 16-bit timer/counter 1 to provide a 1mS tick.
//

#include "PowerMeter.h"

#include "INA219.h"
#include "CharString.h"
#include "StringUtils.h"
#include "Console.h"
#include <avr/io.h>
#include <avr/interrupt.h>

typedef enum PowerMeterState_enum {
    pms_initial,
    pms_waitingForConfigCompletion,
    pms_stopped,
    pms_waitingForTick,
    pms_waitingForRegisterPtrSet,
    pms_waitingForCurrentReading
} PowerMeterState;

static PowerMeterState pmState = pms_initial;

static bool enabled;
static volatile bool tick1mS;
static volatile bool reportIsDue;
static bool INA219OperationComplete;
static volatile uint16_t numTicks;
static uint16_t ticksPerReport; // number of 1mS ticks per report
static uint16_t numSamples;
static int32_t sampleSum;
static int16_t latestCurrentReading;
static volatile int32_t accumulatedTime;    // time in 1mS ticks since last reset
static int32_t accumulatedCurrent;          // power in mAh since last reset
static int16_t adcBias; // compensates for ADC bias

static void writeCompletionHandler (
    const bool success,
    const I2CStatusCode i2cStatus)
{
    INA219OperationComplete = true;
}

static void readCompletionHandler (
    const bool success,
    const I2CStatusCode i2cStatus,
    const int16_t registerValue)
{
    INA219OperationComplete = true;
    latestCurrentReading = registerValue;
}

void PowerMeter_start (void)
{
    enabled = true;
}

void PowerMeter_stop (void)
{
    enabled = false;
}

void PowerMeter_reset (void)
{
    accumulatedCurrent = 0;
    accumulatedTime = 0;
}

void PowerMeter_Initialize (void)
{
    enabled = false;
    ticksPerReport = 100;   // start off with reporting 10 times per second
    accumulatedCurrent = 0;

    adcBias = 5;

    // set up timer1 to fire interrupt every millisecond
    TCCR1B = (TCCR1B & 0xF8) | 3; // prescale by 64
    TCCR1B = (TCCR1B & 0xE7) | (1 << 3); // set CTC mode
    OCR1A = (F_CPU / 64) / 1000;

    pmState = pms_initial;
}

void PowerMeter_task (void)
{
    switch (pmState) {
        case pms_initial :
            if (INA219_setConfiguration(
                false, ibrng_32V, ipga_div8, iadc_12bit, iadc_2sample, im_shuntContinuous,
                writeCompletionHandler)) {
                Console_printP(PSTR("Configuring"));
                pmState = pms_waitingForConfigCompletion;
            }
            break;
        case pms_waitingForConfigCompletion :
            if (INA219OperationComplete) {
                Console_printP(PSTR("Config complete"));
                pmState = pms_stopped;
            }
            break;
        case pms_stopped :
            if (enabled) {
                tick1mS = false;
                reportIsDue = false;
                numTicks = 0;
                numSamples = 0;
                sampleSum = 0;

                Console_printP(PSTR("Starting"));

                // start the timer
                TCNT1 = 0;  // start the time counter at 0
                TIFR1 |= (1 << OCF1A);  // "clear" the timer compare flag
                TIMSK1 |= (1 << OCIE1A);// enable timer compare match interrupt

                INA219OperationComplete = false;
                if (INA219_setRegisterPtr(ira_shuntVoltage, writeCompletionHandler)) {
                    Console_printP(PSTR("setting register ptr"));
                    pmState = pms_waitingForRegisterPtrSet;
                }
            }
            break;
        case pms_waitingForRegisterPtrSet :
            if (INA219OperationComplete) {
                Console_printP(PSTR("waiting for first tick"));
                pmState = pms_waitingForTick;
            }
            break;
        case pms_waitingForTick :
            if (enabled) {
                if (tick1mS) {
                    // request current reading
                    INA219OperationComplete = false;
                    if (INA219_readRegister(readCompletionHandler)) {
                        tick1mS = false;
                        pmState = pms_waitingForCurrentReading;
                    }
                }
            } else {
                // disabled - stop timer interrupts
                TIFR1 |= (1 << OCF1A);  // "clear" the timer compare flag
                TIMSK1 &= ~(1 << OCIE1A);// disable timer compare match interrupt
                pmState = pms_stopped;
            }
            break;
        case pms_waitingForCurrentReading :
            if (INA219OperationComplete) {
                ++numSamples;
                sampleSum += (latestCurrentReading + adcBias);

                if (reportIsDue) {
                    int32_t reportTime;
                    char SREGSave = SREG;
                    cli();
                    reportIsDue = false;
                    reportTime = accumulatedTime;
                    SREG = SREGSave;

                    const int32_t sampleAverageCurrent = sampleSum / numSamples;
                    accumulatedCurrent += sampleAverageCurrent;

                    // report sample and accumulated current
                    CharString_define(40, report);
                    StringUtils_appendDecimal32(reportTime, 1, 3, &report);
                    CharString_appendP(PSTR(", "), &report);
                    StringUtils_appendDecimal32(sampleAverageCurrent, 1, 1, &report);
                    CharString_appendP(PSTR(", "), &report);
                    // compute hundreths of mAh
                    const int32_t accumulatedMAh =
                        accumulatedCurrent / (360 * (1000 / ticksPerReport));
                    StringUtils_appendDecimal32(accumulatedMAh, 1, 2, &report);
                    Console_printCS(&report);

                    // reset for next report
                    numSamples = 0;
                    sampleSum = 0;
                }
                pmState = pms_waitingForTick;
            }
            break;
    }
}

ISR(TIMER1_COMPA_vect)
{
    tick1mS = true;
    ++numTicks;
    if (numTicks >= ticksPerReport) {
        reportIsDue = true;
        numTicks = 0;
    }
    ++accumulatedTime;
}
