#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gamepad.h"


typedef struct {
    uint8_t Id;
    bool Enabled;
    GPAD_TYPE Type;
    uint8_t DataGpio;
} GamepadConfig;

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
    uint8_t NumberOfGamepads;
    GamepadConfig *Gamepads;
    uint8_t ClockGpio;
    uint8_t LatchGpio;
    uint32_t GamepadPollFrequency;
    bool ButtonEnabled;
    uint8_t ButtonGpio;
    uint32_t ButtonPollFrequency;
} SNESDevConfig;


bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, SNESDevConfig *config);