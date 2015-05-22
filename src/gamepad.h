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

#pragma once

#include <stdbool.h>
#include "enum.h"

/* bit masks for checking the button states for SNES controllers */

#define GPAD_SNES_B       0x0001
#define GPAD_SNES_Y       0x0002
#define GPAD_SNES_SELECT  0x0004
#define GPAD_SNES_START   0x0008
#define GPAD_SNES_UP      0x0010
#define GPAD_SNES_DOWN    0x0020
#define GPAD_SNES_LEFT    0x0040
#define GPAD_SNES_RIGHT   0x0080
#define GPAD_SNES_A       0x0100
#define GPAD_SNES_X       0x0200
#define GPAD_SNES_L       0x0400
#define GPAD_SNES_R       0x0800

#define ENUM_GAMEPAD_TYPE(XX) \
    XX(GAMEPAD_NES, =1, nes) \
    XX(GAMEPAD_SNES, =2, snes)

DECLARE_ENUM(GamepadType, ENUM_GAMEPAD_TYPE)

typedef struct {
	uint8_t DataGpio;
	uint16_t State;
} Gamepad;

typedef struct {
	uint8_t ClockGpio;
	uint8_t LatchGpio;
    unsigned int NumberOfGamepads;
    GamepadType Type;
    unsigned int ClockPulses;
} GamepadControlPins;

bool OpenGamepadControlPins(GamepadControlPins *const config);
bool OpenGamepad(Gamepad *const gamepad);
void ReadGamepads(Gamepad *const gamepads, const GamepadControlPins *const config);

