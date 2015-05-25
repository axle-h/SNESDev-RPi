#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gamepad.h"
#include "uinput.h"
#include "button.h"

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    char PidFile[50];

    GamepadsConfig Gamepads;
    ButtonsConfig Buttons;
} SNESDevConfig;


bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, SNESDevConfig *config);