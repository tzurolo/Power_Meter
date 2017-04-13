//
// Command processor
//
// Interprets and executes commands from SMS or the console
//

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "CharString.h"

extern void CommandProcessor_processCommand (
    const char* command);

#endif  // COMMANDPROCESSOR_H