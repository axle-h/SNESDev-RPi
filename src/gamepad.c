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

DEFINE_ENUM(GamepadType, ENUM_GAMEPAD_TYPE, unsigned int)

bool OpenGamepad(GPAD_ST *const gamepad) {
	gamepad->state = 0;

    bool success = false;
    success |= GpioOpen(gamepad->pin_strobe, GPIO_OUTPUT);
    success |= GpioOpen(gamepad->pin_clock, GPIO_OUTPUT);
    success |= GpioOpen(gamepad->pin_data, GPIO_INPUT);

    GpioWrite(gamepad->pin_strobe, GPIO_LOW);
    GpioWrite(gamepad->pin_clock, GPIO_LOW);
	
	return success;
}

void ReadGamepads(GPAD_ST *const gpad) {
	GpioPulse(gpad->pin_strobe);

	gpad->state = 0;
	switch (gpad->type) {
		case GAMEPAD_SNES:
			for (unsigned int i = 0; i < 16; i++) {

				uint8_t curpin1 = GpioRead(gpad->pin_data);
                GpioPulse(gpad->pin_clock);

				if (curpin1 == GPIO_LOW) {
					gpad->state |= (1 << i);
				}
			}

			// set to 0 if the controller is not connected
			if ((gpad->state & 0xFFF) == 0xFFF) {
				gpad->state = 0;
			}
		break;
	case GAMEPAD_NES:
			for (unsigned int i = 0; i < 8; i++) {

				uint8_t curpin1 = GpioRead(gpad->pin_data);
                GpioPulse(gpad->pin_clock);

				if (curpin1 == GPIO_LOW) {
					gpad->state |= (1 << i);
				}
			}

			// set to 0 if the controller is not connected
			if ((gpad->state & 0xFFF) == 0xFFF) {
				gpad->state = 0;
			}
		break;
	default:
		break;
	}

}

