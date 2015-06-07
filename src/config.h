#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gamepad.h"
#include "uinput.h"
#include "button.h"

typedef struct {
    unsigned int Verbose;
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
    int PidFilePointer;
    GamepadsConfig Gamepads;
    ButtonsConfig Buttons;
} SNESDevConfig;


bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, SNESDevConfig *config);