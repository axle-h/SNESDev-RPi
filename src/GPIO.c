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
 
#include <bcm2835.h>
#include <stdbool.h>

#include "GPIO.h"

bool GpioOpen(uint8_t pin, GpioDirection direction)
{
	switch (direction){
		case GPIO_OUTPUT:
			bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
            bcm2835_gpio_write(pin, LOW);
			return true;
		case GPIO_INPUT:
			// Output needs a pull down for when controller not connected.
			bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
			bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_DOWN);
			return true;
	}

	return false;
}

GpioLevel GpioRead(uint8_t pin) {
	return (GpioLevel)bcm2835_gpio_lev(pin);
}

void GpioWrite(uint8_t pin, GpioLevel val) {
    bcm2835_gpio_write(pin, val);
}

void GpioPulse(uint8_t pin, uint64_t micros) {
    bcm2835_gpio_write(pin, HIGH);
	bcm2835_delayMicroseconds(micros);
    bcm2835_gpio_write(pin, LOW);
	bcm2835_delayMicroseconds(micros);
}
