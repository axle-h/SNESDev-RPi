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

typedef enum {
    GPIO_OUTPUT = 0,
    GPIO_INPUT  = 1,
    GPIO_INPUT_LOW  = 2,
    GPIO_INPUT_HIGH  = 3
} GpioDirection;

typedef enum {
    GPIO_LOW  = 0x0,
    GPIO_HIGH = 0x1
} GpioLevel;

bool GpioOpen(uint8_t pin, GpioDirection direction);
GpioLevel GpioRead(uint8_t pin);
void GpioWrite(uint8_t pin, GpioLevel val);
void GpioPulseHigh(uint8_t pin, uint64_t microsHigh, uint64_t microsLow);
void GpioPulseLow(uint8_t pin, uint64_t microsLow, uint64_t microsHigh);
