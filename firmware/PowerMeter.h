//
// Power Meter - using INA219 high side current sensor
//
//  How to use it
//

#ifndef POWERMETER_H
#define POWERMETER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void PowerMeter_start (void);

extern void PowerMeter_stop (void);

extern void PowerMeter_reset (void);

extern void PowerMeter_Initialize (void);

extern void PowerMeter_task (void);

#endif  // POWERMETER_H