/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the USBtoSerial project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "USBtoSerial.h"
#include "USBTerminal.h"
#include "SystemTime.h"
#include "I2CAsync.h"
#include "PowerMeter.h"
#include "StringUtils.h"
#include "Console.h"

#include <avr/pgmspace.h>

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void Initialize (void)
{
    // enable watchdog timer
    wdt_enable(WDTO_500MS);

    /* Disable clock division */
//	clock_prescale_set(clock_div_1);

    Console_Initialize();
    SystemTime_Initialize();
    I2CAsync_Initialize();
    PowerMeter_Initialize();
    USBTerminal_Initialize();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    Initialize();
    sei();

    for (;;) {
        // run all the tasks
        SystemTime_task();
        PowerMeter_task();
        I2CAsync_task();
        USBTerminal_task();
        Console_task();
    }
}

