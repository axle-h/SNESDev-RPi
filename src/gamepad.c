/*
 * SNESDev - User-space driver for the RetroPie GPIO Adapter for the Raspberry Pi.
 *
 * (c) Copyright 2012-2013  Florian Müller (contact@petrockblock.com)
 *
 * SNESDev homepage: https://github.com/petrockblog/SNESDev-RPi
 *
 * Permission to use, copy, modify and distribute SNESDev in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * SNESDev is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for SNESDev or software derived from SNESDev.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Raspberry Pi is a trademark of the Raspberry Pi Foundation.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "gamepad.h"
#include "GPIO.h"

#define SNES_CLOCK 16
#define NES_CLOCK 8

// It's impossible to press up and down at the same time
#define NOT_CONNECTED_MASK (GPAD_SNES_UP | GPAD_SNES_DOWN)

DEFINE_ENUM(GamepadType, ENUM_GAMEPAD_TYPE, unsigned int)

bool OpenGamepadControlPins(GamepadControlPins *const config) {
    bool success = GpioOpen(config->LatchGpio, GPIO_OUTPUT) && GpioOpen(config->ClockGpio, GPIO_OUTPUT);
    GpioWrite(config->LatchGpio, GPIO_LOW);
    GpioWrite(config->ClockGpio, GPIO_LOW);

    switch (config->Type) {
        case GAMEPAD_NES:
            config->ClockPulses = NES_CLOCK;
            break;
        case GAMEPAD_SNES:
            config->ClockPulses = SNES_CLOCK;
            break;
    }

    return success;
}

bool OpenGamepad(Gamepad *const gamepad) {
	gamepad->State = 0;
    return GpioOpen(gamepad->DataGpio, GPIO_INPUT);
}

void ReadGamepads(Gamepad *const gamepads, const GamepadControlPins *const config) {
    if(config->NumberOfGamepads == 0) {
        return;
    }

	GpioPulse(config->LatchGpio);

    for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
        Gamepad *gamepad = &gamepads[i];
        gamepad->State = 0;
    }

	for (unsigned int clock = 0; clock < config->ClockPulses; clock++) {
		for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
			Gamepad *gamepad = &gamepads[i];

			if (GpioRead(gamepad->DataGpio) == GPIO_LOW) {
                gamepad->State |= (1 << i);
			}
		}

		GpioPulse(config->ClockGpio);
	}

    for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
        Gamepad *gamepad = &gamepads[i];

        // set to 0 if the controller is not connected
        if ((gamepad->State & NOT_CONNECTED_MASK) == NOT_CONNECTED_MASK) {
            gamepad->State = 0;
        }
    }
}

