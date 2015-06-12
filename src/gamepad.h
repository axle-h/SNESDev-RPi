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
#include "uinput.h"
#include "SNESDevConfig.h"


#define ENUM_GAMEPAD_BUTTON(XX) \
    XX(GAMEPAD_BUTTON_B, =0x0001, B) \
    XX(GAMEPAD_BUTTON_Y, =0x0002, Y) \
    XX(GAMEPAD_BUTTON_SELECT, =0x0004, SELECT) \
    XX(GAMEPAD_BUTTON_START, =0x0008, START) \
    XX(GAMEPAD_BUTTON_UP, =0x0010, UP) \
    XX(GAMEPAD_BUTTON_DOWN, =0x0020, DOWN) \
    XX(GAMEPAD_BUTTON_LEFT, =0x0040, LEFT) \
    XX(GAMEPAD_BUTTON_RIGHT, =0x0080, RIGHT) \
    XX(GAMEPAD_BUTTON_A, =0x0100, A) \
    XX(GAMEPAD_BUTTON_X, =0x0200, X) \
    XX(GAMEPAD_BUTTON_L, =0x0400, L) \
    XX(GAMEPAD_BUTTON_R, =0x0800, R) \

// Gamepad & clock pulses
#define ENUM_GAMEPAD_TYPE(XX) \
    XX(GAMEPAD_NES, =1, nes) \
    XX(GAMEPAD_SNES, =2, snes)

DECLARE_ENUM(GamepadType, ENUM_GAMEPAD_TYPE)
DECLARE_ENUM(GamepadButton, ENUM_GAMEPAD_BUTTON)

typedef struct {
    unsigned int Id;
    uint8_t DataGpio;
} GamepadConfig;

typedef struct {
    unsigned int Total;
    GamepadConfig Gamepads[SNESDEV_MAX_GAMEPADS];
    GamepadType Type;
    unsigned int ClockPulses;
    uint8_t ClockGpio;
    uint8_t LatchGpio;
    unsigned int PollFrequency;
} GamepadsConfig;

typedef struct {
    uint8_t DataGpio;
    uint16_t State;
    uint16_t LastState;
    bool B;
    bool Y;
    bool Select;
    bool Start;
    DigitalAxisValue YAxis;
    DigitalAxisValue XAxis;
    bool A;
    bool X;
    bool L;
    bool R;
} Gamepad;

bool OpenGamepadControlPins(GamepadsConfig *config);
bool OpenGamepad(Gamepad *gamepad);
void ReadGamepads(Gamepad *gamepads, GamepadsConfig *config);
bool CheckGamepadState(Gamepad *gamepad);

