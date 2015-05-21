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

#define ENUM_VIRTUAL_KEYS(XX) \
    XX(VIRTUAL_KEY_1, =2, 1) \
    XX(VIRTUAL_KEY_2, =3, 2) \
    XX(VIRTUAL_KEY_3, =4, 3) \
    XX(VIRTUAL_KEY_4, =5, 4) \
    XX(VIRTUAL_KEY_5, =6, 5) \
    XX(VIRTUAL_KEY_6, =7, 6) \
    XX(VIRTUAL_KEY_7, =8, 7) \
    XX(VIRTUAL_KEY_8, =9, 8) \
    XX(VIRTUAL_KEY_9, =10, 9) \
    XX(VIRTUAL_KEY_0, =11, 0) \
    XX(VIRTUAL_KEY_MINUS, =12, MINUS) \
    XX(VIRTUAL_KEY_EQUAL, =13, EQUAL) \
    XX(VIRTUAL_KEY_Q, =16, Q) \
    XX(VIRTUAL_KEY_W, =17, W) \
    XX(VIRTUAL_KEY_E, =18, E) \
    XX(VIRTUAL_KEY_R, =19, R) \
    XX(VIRTUAL_KEY_T, =20, T) \
    XX(VIRTUAL_KEY_Y, =21, Y) \
    XX(VIRTUAL_KEY_U, =22, U) \
    XX(VIRTUAL_KEY_I, =23, I) \
    XX(VIRTUAL_KEY_O, =24, O) \
    XX(VIRTUAL_KEY_P, =25, P) \
    XX(VIRTUAL_KEY_A, =30, A) \
    XX(VIRTUAL_KEY_S, =31, S) \
    XX(VIRTUAL_KEY_D, =32, D) \
    XX(VIRTUAL_KEY_F, =33, F) \
    XX(VIRTUAL_KEY_G, =34, G) \
    XX(VIRTUAL_KEY_H, =35, H) \
    XX(VIRTUAL_KEY_J, =36, J) \
    XX(VIRTUAL_KEY_K, =37, K) \
    XX(VIRTUAL_KEY_L, =38, L) \
    XX(VIRTUAL_KEY_SEMICOLON, =39, SEMICOLON) \
    XX(VIRTUAL_KEY_APOSTROPHE, =40, APOSTROPHE) \
    XX(VIRTUAL_KEY_Z, =44, Z) \
    XX(VIRTUAL_KEY_X, =45, X) \
    XX(VIRTUAL_KEY_C, =46, C) \
    XX(VIRTUAL_KEY_V, =47, V) \
    XX(VIRTUAL_KEY_B, =48, B) \
    XX(VIRTUAL_KEY_N, =49, N) \
    XX(VIRTUAL_KEY_M, =50, M) \
    XX(VIRTUAL_KEY_COMMA, =51, COMMA) \
    XX(VIRTUAL_KEY_DOT, =52, DOT) \
    XX(VIRTUAL_KEY_SLASH, =53, SLASH) \
    XX(VIRTUAL_KEY_SPACE, =57, SPACE) \
    XX(VIRTUAL_KEY_ESC, =1, ESC) \
    XX(VIRTUAL_KEY_BACKSPACE, =14, BACKSPACE) \
    XX(VIRTUAL_KEY_ENTER, =28, ENTER) \
    XX(VIRTUAL_KEY_F1, =59, F1) \
    XX(VIRTUAL_KEY_F2, =60, F2) \
    XX(VIRTUAL_KEY_F3, =61, F3) \
    XX(VIRTUAL_KEY_F4, =62, F4) \
    XX(VIRTUAL_KEY_F5, =63, F5) \
    XX(VIRTUAL_KEY_F6, =64, F6) \
    XX(VIRTUAL_KEY_F7, =65, F7) \
    XX(VIRTUAL_KEY_F8, =66, F8) \
    XX(VIRTUAL_KEY_F9, =67, F9) \
    XX(VIRTUAL_KEY_F10, =68, F10) \
    XX(VIRTUAL_KEY_F11, =87, F11) \
    XX(VIRTUAL_KEY_F12, =88, F12)


DECLARE_ENUM(VirtualKey, ENUM_VIRTUAL_KEYS)


typedef enum {
    Gamepad,
    Keyboard
} InputDeviceType;

typedef struct {
	int File;
    char *Name;
} InputDevice;

bool OpenInputDevice(const InputDeviceType deviceType, InputDevice *const device);
bool CloseInputDevice(InputDevice *const device);
bool WriteKey(InputDevice *const device, unsigned short int key, bool keyPressed);
bool WriteAxis(InputDevice *const device, unsigned short int axis, int value);
bool WriteSync(InputDevice *const device);
