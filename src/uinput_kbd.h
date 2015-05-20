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

#define VIRTUAL_KEYS(XX) \
    XX(VIRTUAL_KEY_1,=KEY_1) \
    XX(VIRTUAL_KEY_2,=KEY_2) \
    XX(VIRTUAL_KEY_3,=KEY_3) \
    XX(VIRTUAL_KEY_4,=KEY_4) \
    XX(VIRTUAL_KEY_5,=KEY_5) \
    XX(VIRTUAL_KEY_6,=KEY_6) \
    XX(VIRTUAL_KEY_7,=KEY_7) \
    XX(VIRTUAL_KEY_8,=KEY_8) \
    XX(VIRTUAL_KEY_9,=KEY_9) \
    XX(VIRTUAL_KEY_0,=KEY_0) \
    XX(VIRTUAL_KEY_MINUS,=KEY_MINUS) \
    XX(VIRTUAL_KEY_EQUAL,=KEY_EQUAL) \
    XX(VIRTUAL_KEY_Q,=KEY_Q) \
    XX(VIRTUAL_KEY_W,=KEY_W) \
    XX(VIRTUAL_KEY_E,=KEY_E) \
    XX(VIRTUAL_KEY_R,=KEY_R) \
    XX(VIRTUAL_KEY_T,=KEY_T) \
    XX(VIRTUAL_KEY_Y,=KEY_Y) \
    XX(VIRTUAL_KEY_U,=KEY_U) \
    XX(VIRTUAL_KEY_I,=KEY_I) \
    XX(VIRTUAL_KEY_O,=KEY_O) \
    XX(VIRTUAL_KEY_P,=KEY_P) \
    XX(VIRTUAL_KEY_A,=KEY_A) \
    XX(VIRTUAL_KEY_S,=KEY_S) \
    XX(VIRTUAL_KEY_D,=KEY_D) \
    XX(VIRTUAL_KEY_F,=KEY_F) \
    XX(VIRTUAL_KEY_G,=KEY_G) \
    XX(VIRTUAL_KEY_H,=KEY_H) \
    XX(VIRTUAL_KEY_J,=KEY_J) \
    XX(VIRTUAL_KEY_K,=KEY_K) \
    XX(VIRTUAL_KEY_L,=KEY_L) \
    XX(VIRTUAL_KEY_SEMICOLON,=KEY_SEMICOLON) \
    XX(VIRTUAL_KEY_APOSTROPHE,=KEY_APOSTROPHE) \
    XX(VIRTUAL_KEY_Z,=KEY_Z) \
    XX(VIRTUAL_KEY_X,=KEY_X) \
    XX(VIRTUAL_KEY_C,=KEY_C) \
    XX(VIRTUAL_KEY_V,=KEY_V) \
    XX(VIRTUAL_KEY_B,=KEY_B) \
    XX(VIRTUAL_KEY_N,=KEY_N) \
    XX(VIRTUAL_KEY_M,=KEY_M) \
    XX(VIRTUAL_KEY_COMMA,=KEY_COMMA) \
    XX(VIRTUAL_KEY_DOT,=KEY_DOT) \
    XX(VIRTUAL_KEY_SLASH,=KEY_SLASH) \
    XX(VIRTUAL_KEY_SPACE,=KEY_SPACE) \
    XX(VIRTUAL_KEY_ESC,=KEY_ESC) \
    XX(VIRTUAL_KEY_BACKSPACE,=KEY_BACKSPACE) \
    XX(VIRTUAL_KEY_ENTER,=KEY_ENTER) \
    XX(VIRTUAL_KEY_F1,=KEY_F1) \
    XX(VIRTUAL_KEY_F2,=KEY_F2) \
    XX(VIRTUAL_KEY_F3,=KEY_F3) \
    XX(VIRTUAL_KEY_F4,=KEY_F4) \
    XX(VIRTUAL_KEY_F5,=KEY_F5) \
    XX(VIRTUAL_KEY_F6,=KEY_F6) \
    XX(VIRTUAL_KEY_F7,=KEY_F7) \
    XX(VIRTUAL_KEY_F8,=KEY_F8) \
    XX(VIRTUAL_KEY_F9,=KEY_F9) \
    XX(VIRTUAL_KEY_F10,=KEY_F10) \
    XX(VIRTUAL_KEY_F11,=KEY_F11) \
    XX(VIRTUAL_KEY_F12,=KEY_F12) \

DECLARE_ENUM(VirtualKey, VIRTUAL_KEYS)


typedef struct {
	int16_t FileDescriptor;
} VirtualKeyboard;

bool OpenVirtualKeyboard(VirtualKeyboard *const keyboard);
bool CloseVirtualKeyboard(VirtualKeyboard *const keyboard);
bool WriteToVirtualKeyboard(VirtualKeyboard *const keyboard, unsigned short int key, bool keyPressed);

