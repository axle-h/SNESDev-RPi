#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gamepad.h"


typedef struct {
    unsigned int Id;
    bool Enabled;
    GamepadType Type;
    uint8_t DataGpio;
} GamepadConfig;

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
    unsigned int NumberOfGamepads;
    GamepadConfig *Gamepads;
    uint8_t ClockGpio;
    uint8_t LatchGpio;
    unsigned int GamepadPollFrequency;
    bool ButtonEnabled;
    uint8_t ButtonGpio;
    unsigned int ButtonPollFrequency;
} SNESDevConfig;


bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, const unsigned int maxGamepads, SNESDevConfig *config);