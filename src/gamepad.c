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

#define SNES_CLOCK 12
#define NES_CLOCK 8

DEFINE_ENUM(GamepadType, ENUM_GAMEPAD_TYPE, unsigned int)
DEFINE_ENUM(GamepadButton, ENUM_GAMEPAD_BUTTON, unsigned int)

bool OpenGamepadControlPins(GamepadsConfig *const config) {
    bool success = GpioOpen(config->LatchGpio, GPIO_OUTPUT) && GpioOpen(config->ClockGpio, GPIO_OUTPUT);
    GpioWrite(config->ClockGpio, GPIO_HIGH);

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
    gamepad->XAxis = DIGITAL_AXIS_ORIGIN;
    gamepad->YAxis = DIGITAL_AXIS_ORIGIN;
    return GpioOpen(gamepad->DataGpio, GPIO_INPUT_HIGH);
}

void ReadGamepads(Gamepad *const gamepads, GamepadsConfig *const config) {

    Gamepad *gamepad;
    for(unsigned int i = 0; i < config->Total; i++) {
        gamepad = gamepads + i;
        gamepad->LastState = gamepad->State;
        gamepad->State = 0;
    }

    // Latch the shift register.
    GpioPulseHigh(config->LatchGpio, 12, 6);

	for (unsigned int clock = 0; clock < config->ClockPulses; clock++) {
		for(unsigned int i = 0; i < config->Total; i++) {
			gamepad = gamepads + i;

            // SNES sets gpio low when button pressed.
            // Must have a pull-up resistor or we'll get all buttons pressed when controller disconnected.
			if (GpioRead(gamepad->DataGpio) == GPIO_LOW) {
                gamepad->State |= (1 << clock);
			}
		}

        // Pulse the clock to shift the register
		GpioPulseLow(config->ClockGpio, 6, 6);
	}
}

bool CheckGamepadState(Gamepad *const gamepad) {
    if(gamepad->LastState == gamepad->State) {
        return false;
    }

    gamepad->B = (gamepad->State & GAMEPAD_BUTTON_B) != 0;
    gamepad->Y = (gamepad->State & GAMEPAD_BUTTON_Y) != 0;
    gamepad->Select = (gamepad->State & GAMEPAD_BUTTON_SELECT) != 0;
    gamepad->Start = (gamepad->State & GAMEPAD_BUTTON_START) != 0;

    bool up = (gamepad->State & GAMEPAD_BUTTON_UP) != 0;
    bool down = (gamepad->State & GAMEPAD_BUTTON_DOWN) != 0;
    bool left = (gamepad->State & GAMEPAD_BUTTON_LEFT) != 0;
    bool right = (gamepad->State & GAMEPAD_BUTTON_RIGHT) != 0;

    gamepad->YAxis = up ? DIGITAL_AXIS_HIGH
                        : down ? DIGITAL_AXIS_LOW
                        : DIGITAL_AXIS_ORIGIN;

    gamepad->XAxis = left ? DIGITAL_AXIS_HIGH
                        : right ? DIGITAL_AXIS_LOW
                        : DIGITAL_AXIS_ORIGIN;

    gamepad->A = (gamepad->State & GAMEPAD_BUTTON_A) != 0;
    gamepad->X = (gamepad->State & GAMEPAD_BUTTON_X) != 0;
    gamepad->L = (gamepad->State & GAMEPAD_BUTTON_L) != 0;
    gamepad->R = (gamepad->State & GAMEPAD_BUTTON_R) != 0;

    // Check that we don't have noise.
    if((up && down) || (left && right)) {
        gamepad->State = 0;
        return false;
    }

    return true;
}