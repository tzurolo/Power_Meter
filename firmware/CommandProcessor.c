//
// Command processor
//
// Interprets and executes commands from SMS or the console
//

#include "CommandProcessor.h"

#include <stdlib.h>
#include <avr/pgmspace.h>
#include "USBTerminal.h"
#include "SystemTime.h"
#include "EEPROM.h"
#include "Console.h"
#include "I2CAsync.h"
#include "StringUtils.h"
#include "PowerMeter.h"

#define CMD_TOKEN_BUFFER_LEN 80

static const char tokenDelimiters[] = " \n\r";

// appends n.nnV to outgoing message text
static void appendVoltageToString (
    const int16_t voltage,
    CharString_t *str)
{
    StringUtils_appendDecimal(voltage, 1, 2, str);
    CharString_appendP(PSTR("V"), str);
}

void CommandProcessor_processCommand (
    const char* command)
{
#if 0
	char msgbuf[80];
    sprintf(msgbuf, "cmd: '%s'", command);
    Console_print(msgbuf);
    sprintf(msgbuf, "phone#: '%s'", phoneNumber);
    Console_print(msgbuf);
    sprintf(msgbuf, "time: '%s'", timestamp);
    Console_print(msgbuf);
#endif

    char cmdTokenBuf[CMD_TOKEN_BUFFER_LEN];
    strncpy(cmdTokenBuf, command, CMD_TOKEN_BUFFER_LEN-1);
    cmdTokenBuf[CMD_TOKEN_BUFFER_LEN-1] = 0;
    const char* cmdToken = strtok(cmdTokenBuf, tokenDelimiters);
    if (cmdToken != NULL) {
        if (strcasecmp_P(cmdToken, PSTR("reset")) == 0) {
            PowerMeter_reset();
        } else if (strcasecmp_P(cmdToken, PSTR("start")) == 0) {
            PowerMeter_start();
        } else if (strcasecmp_P(cmdToken, PSTR("stop")) == 0) {
            PowerMeter_stop();
        } else if (strcasecmp_P(cmdToken, PSTR("sample")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                const unsigned int samplesPerSec = atoi(cmdToken);
                Console_printP(PSTR("samples"));
            }
        } else if (strcasecmp_P(cmdToken, PSTR("report")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                const unsigned int reportsPerSec = atoi(cmdToken);
                Console_printP(PSTR("reports"));
            }
	} else if (strcasecmp_P(cmdToken, PSTR("eeread")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                const unsigned int uiAddress = atoi(cmdToken);
                const uint8_t eeromData = EEPROM_read(uiAddress);
                CharString_define(30, eeromStr);
                StringUtils_appendDecimal(uiAddress, 1, 0, &eeromStr);
                CharString_appendP(PSTR(":"), &eeromStr);
                StringUtils_appendDecimal(eeromData, 1, 0, &eeromStr);
                Console_printCS(&eeromStr);
            }
        } else if (strcasecmp_P(cmdToken, PSTR("eewrite")) == 0) {
            cmdToken = strtok(NULL, tokenDelimiters);
            if (cmdToken != NULL) {
                const unsigned int uiAddress = atoi(cmdToken);
                cmdToken = strtok(NULL, tokenDelimiters);
                if (cmdToken != NULL) {
                    const uint8_t value = atoi(cmdToken);
                    EEPROM_write(uiAddress, value);
                }
            }
        } else {
            Console_printP(PSTR("unrecognized command"));
        }
    }
}
//                uint32_t i1 = 30463UL;
//                uint32_t i2 = 30582UL;
