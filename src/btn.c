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
 
#include "btn.h"
#include "GPIO.h"

int16_t btn_open(BTN_DEV_ST * btn) {
	btn->state = BTN_STATE_IDLE;
	return gpio_open(btn->pin, GPIO_INPUT);
}

void btn_read(BTN_DEV_ST* const btn) {
	// read the state of the button into a local variable
	uint8_t buttonState = gpio_read_pin(btn->pin);

	// three-state machine:
	// - press and hold: send "r" key (for rewind function of RetroArch)
	// - press and release three times: send "ESC"
	// - press and release five times: shutdown
	switch (btn->state) {
	case BTN_STATE_IDLE:
		if (buttonState == GPIO_HIGH) {
			btn->state = BTN_STATE_PRESSED;
		}
		break;
	case BTN_STATE_PRESSED:
		if (buttonState == GPIO_LOW) {
			btn->state = BTN_STATE_RELEASED;
		}
		break;
	case BTN_STATE_RELEASED:
		if (buttonState == GPIO_LOW) {
			btn->state = BTN_STATE_IDLE;
		} else if (buttonState == GPIO_HIGH) {
			btn->state = BTN_STATE_PRESSED;
		}
		break;
	}
}
