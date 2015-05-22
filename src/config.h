#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gamepad.h"
#include "uinput.h"


typedef struct {
    unsigned int Id;
    bool Enabled;
    uint8_t DataGpio;
} GamepadConfig;

typedef struct {
    unsigned int Id;
    bool Enabled;
    InputKey Key;
    uint8_t DataGpio;
} ButtonConfig;

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;

    unsigned int NumberOfGamepads;
    GamepadConfig *Gamepads;
    GamepadType Type;
    uint8_t ClockGpio;
    uint8_t LatchGpio;
    unsigned int GamepadPollFrequency;

    unsigned int NumberOfButtons;
    ButtonConfig *Buttons;
    unsigned int ButtonPollFrequency;
} SNESDevConfig;


bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, const unsigned int maxGamepads, SNESDevConfig *config);