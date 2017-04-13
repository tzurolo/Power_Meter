//
//  Console for USB host
//
//
#include "Console.h"

#include "USBTerminal.h"
#include "SystemTime.h"
#include "CommandProcessor.h"
#include "StringUtils.h"
#include "EEPROM.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

#define SINGLE_SCREEN 0

#define ANSI_ESCAPE_SEQUENCE(EscapeSeq)  "\33[" EscapeSeq
#define ESC_CURSOR_POS(Line, Column)    ANSI_ESCAPE_SEQUENCE(#Line ";" #Column "H")
#define ESC_ERASE_LINE                  ANSI_ESCAPE_SEQUENCE("K")
#define ESC_CURSOR_POS_RESTORE          ANSI_ESCAPE_SEQUENCE("u")

// state variables
CharString_define(40, commandBuffer)
static SystemTime_t nextStatusPrintTime;
static uint8_t currentPrintLine = 5;

void Console_Initialize (void)
{
}

void Console_task (void)
{
    const uint16_t bufferCount = ByteQueue_length(&FromUSB_Buffer);
    if (bufferCount > 0) {
        const uint8_t cmdByte = ByteQueue_pop(&FromUSB_Buffer);
        switch (cmdByte) {
            case '\r' : {
                // command complete. execute it
#if SINGLE_SCREEN
                USBTerminal_sendCharsToHost(ESC_CURSOR_POS(2, 1));
                USBTerminal_sendCharsToHostCS(&commandBuffer);
#else
                USBTerminal_sendCharsToHost(ESC_ERASE_LINE);
                USBTerminal_sendCharsToHost("\r\n");
#endif
                CommandProcessor_processCommand(CharString_cstr(&commandBuffer));
                CharString_clear(&commandBuffer);
                }
                break;
            case 0x7f : {
                CharString_truncate(CharString_length(&commandBuffer) - 1,
                    &commandBuffer);
                }
                break;
            default : {
                // command not complete yet. append to command buffer
                CharString_appendC(cmdByte, &commandBuffer);
                }
                break;
        }
		// echo current command
#if SINGLE_SCREEN
        USBTerminal_sendCharsToHost(ESC_CURSOR_POS(1, 1));
#else
        USBTerminal_sendCharsToHost("\r");
#endif
        USBTerminal_sendCharsToHostCS(&commandBuffer);

        USBTerminal_sendCharsToHost(ESC_ERASE_LINE);

    }

    // display status
    if (USBTerminal_isConnected () &&
        SystemTime_timeHasArrived(&nextStatusPrintTime)) {
#if SINGLE_SCREEN
        USBTerminal_sendCharsToHost(ESC_CURSOR_POS(3, 1));
#endif
        CharString_define(120, statusMsg)
        CharString_append("status...", &statusMsg);
        //USBTerminal_sendLineToHostCS(&statusMsg);

	// schedule next display
	SystemTime_futureTime(1, &nextStatusPrintTime);
	}
}

static void sendCursorTo (
    const int line,
    const int column)
{
    CharString_define(16, cursorToBuf);
    CharString_copyP(PSTR("\33["), &cursorToBuf);
    StringUtils_appendDecimal(line, 1, 0, &cursorToBuf);
    CharString_appendC(';', &cursorToBuf);
    StringUtils_appendDecimal(column, 1, 0, &cursorToBuf);
    CharString_appendC('H', &cursorToBuf);
    USBTerminal_sendCharsToHostCS(&cursorToBuf);
}

void Console_print (
    const char* text)
{
    if (USBTerminal_isConnected ()) {
#if SINGLE_SCREEN
#if 0
        if (currentPrintLine > 22) {
            currentPrintLine = 5;
            USBTerminal_sendCharsToHost(ESC_CURSOR_POS(5, 1));
            USBTerminal_sendCharsToHost(ANSI_ESCAPE_SEQUENCE("J"));
        }
#endif
        // print text on the current line
	sendCursorTo(currentPrintLine, 1);
#endif
        USBTerminal_sendLineToHost(text);
#if SINGLE_SCREEN
        USBTerminal_sendCharsToHost(ESC_CURSOR_POS_RESTORE);
        ++currentPrintLine;

        // restore cursor to command buffer end
        sendCursorTo(1, CharString_length(&commandBuffer)+1);
#endif
    }
}

void Console_printP (
    PGM_P text)
{
    if (USBTerminal_isConnected ()) {
#if SINGLE_SCREEN
#if 0
        if (currentPrintLine > 22) {
            currentPrintLine = 5;
            USBTerminal_sendCharsToHost(ESC_CURSOR_POS(5, 1));
            USBTerminal_sendCharsToHost(ANSI_ESCAPE_SEQUENCE("J"));
        }
#endif

	// print text on the current line
	sendCursorTo(currentPrintLine, 1);
#endif
        USBTerminal_sendLineToHostP(text);
#if SINGLE_SCREEN
        USBTerminal_sendCharsToHost(ESC_CURSOR_POS_RESTORE);
        ++currentPrintLine;

        // restore cursor to command buffer end
        sendCursorTo(1, CharString_length(&commandBuffer)+1);
#endif
    }
}

void Console_printCS (
    const CharString_t *text)
{
    if (USBTerminal_isConnected ()) {
#if SINGLE_SCREEN
#if 0
        if (currentPrintLine > 22) {
	        currentPrintLine = 5;
	        USBTerminal_sendCharsToHost(ESC_CURSOR_POS(5, 1));
	        USBTerminal_sendCharsToHost(ANSI_ESCAPE_SEQUENCE("J"));
        }
#endif
        // print text on the current line
        sendCursorTo(currentPrintLine, 1);
#endif
        USBTerminal_sendLineToHostCS(text);
#if SINGLE_SCREEN
        USBTerminal_sendCharsToHost(ESC_CURSOR_POS_RESTORE);
        ++currentPrintLine;

        // restore cursor to command buffer end
        sendCursorTo(1, CharString_length(&commandBuffer)+1);
#endif
    }
}
