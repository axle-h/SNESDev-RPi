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

#include <stdint.h>
#include <stdbool.h>
#include <linux/input.h>
#include "enum.h"

#define ENUM_INPUT_KEYS(XX) \
    XX(INPUT_KEY_1, =2, 1) \
    XX(INPUT_KEY_2, =3, 2) \
    XX(INPUT_KEY_3, =4, 3) \
    XX(INPUT_KEY_4, =5, 4) \
    XX(INPUT_KEY_5, =6, 5) \
    XX(INPUT_KEY_6, =7, 6) \
    XX(INPUT_KEY_7, =8, 7) \
    XX(INPUT_KEY_8, =9, 8) \
    XX(INPUT_KEY_9, =10, 9) \
    XX(INPUT_KEY_0, =11, 0) \
    XX(INPUT_KEY_Q, =16, Q) \
    XX(INPUT_KEY_W, =17, W) \
    XX(INPUT_KEY_E, =18, E) \
    XX(INPUT_KEY_R, =19, R) \
    XX(INPUT_KEY_T, =20, T) \
    XX(INPUT_KEY_Y, =21, Y) \
    XX(INPUT_KEY_U, =22, U) \
    XX(INPUT_KEY_I, =23, I) \
    XX(INPUT_KEY_O, =24, O) \
    XX(INPUT_KEY_P, =25, P) \
    XX(INPUT_KEY_A, =30, A) \
    XX(INPUT_KEY_S, =31, S) \
    XX(INPUT_KEY_D, =32, D) \
    XX(INPUT_KEY_F, =33, F) \
    XX(INPUT_KEY_G, =34, G) \
    XX(INPUT_KEY_H, =35, H) \
    XX(INPUT_KEY_J, =36, J) \
    XX(INPUT_KEY_K, =37, K) \
    XX(INPUT_KEY_L, =38, L) \
    XX(INPUT_KEY_Z, =44, Z) \
    XX(INPUT_KEY_X, =45, X) \
    XX(INPUT_KEY_C, =46, C) \
    XX(INPUT_KEY_V, =47, V) \
    XX(INPUT_KEY_B, =48, B) \
    XX(INPUT_KEY_N, =49, N) \
    XX(INPUT_KEY_M, =50, M) \
    XX(INPUT_KEY_SPACE, =57, SPACE) \
    XX(INPUT_KEY_ESC, =1, ESC) \
    XX(INPUT_KEY_BACKSPACE, =14, BACKSPACE) \
    XX(INPUT_KEY_ENTER, =28, ENTER) \
    XX(INPUT_KEY_F1, =59, F1) \
    XX(INPUT_KEY_F2, =60, F2) \
    XX(INPUT_KEY_F3, =61, F3) \
    XX(INPUT_KEY_F4, =62, F4) \
    XX(INPUT_KEY_F5, =63, F5) \
    XX(INPUT_KEY_F6, =64, F6) \
    XX(INPUT_KEY_F7, =65, F7) \
    XX(INPUT_KEY_F8, =66, F8) \
    XX(INPUT_KEY_F9, =67, F9) \
    XX(INPUT_KEY_F10, =68, F10) \
    XX(INPUT_KEY_F11, =87, F11) \
    XX(INPUT_KEY_F12, =88, F12)


DECLARE_ENUM(InputKey, ENUM_INPUT_KEYS)


typedef enum {
    INPUT_GAMEPAD,
    INPUT_KEYBOARD
} InputDeviceType;

typedef struct {
	int File;
    const char *Name;
} InputDevice;

bool OpenInputDevice(const InputDeviceType deviceType, InputDevice *const device);
bool CloseInputDevice(InputDevice *const device);
bool WriteKey(InputDevice *const device, unsigned short int key, bool keyPressed);
bool WriteAxis(InputDevice *const device, unsigned short int axis, int value);
bool WriteSync(InputDevice *const device);
