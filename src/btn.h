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

typedef enum {
	BTN_STATE_IDLE = 0, BTN_STATE_RELEASED = 1, BTN_STATE_PRESSED = 2
} BTN_STATE_E;

typedef struct {
	int16_t port;
	int16_t pin;
	BTN_STATE_E state;
	uint32_t duration;
	uint16_t pressedCtr;
	uint32_t lastPress;
} BTN_DEV_ST;

int16_t btn_open(BTN_DEV_ST* btn);
void btn_read(BTN_DEV_ST* const btn);
int16_t btn_ioctlr();

