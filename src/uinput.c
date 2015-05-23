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
 
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uinput.h"


#define UINPUT_DEVICE "/dev/uinput"

DEFINE_ENUM(InputKey, ENUM_INPUT_KEYS, unsigned int)

bool OpenInputDevice(const InputDeviceType deviceType, InputDevice *const device)
{
	device->File = open(UINPUT_DEVICE, O_WRONLY | O_NDELAY);
	if (device->File < 0) {
		fprintf(stderr, "Unable to open %s\n", UINPUT_DEVICE);
		return false;
	}

	struct uinput_user_dev userInput;
	memset(&userInput, 0, sizeof(userInput));
	strncpy(userInput.name, device->Name, strlen(device->Name));
    userInput.id.version = 4;
    userInput.id.bustype = BUS_USB;
    userInput.id.product = 1;
    userInput.id.vendor = 1;

	// Setup the uinput device as a keyboard with basic keys
	ioctl(device->File, UI_SET_EVBIT, EV_KEY);
	ioctl(device->File, UI_SET_EVBIT, EV_REL);

    switch (deviceType) {
        case INPUT_GAMEPAD:
            // Buttons.
            ioctl(device->File, UI_SET_KEYBIT, BTN_A);
            ioctl(device->File, UI_SET_KEYBIT, BTN_B);
            ioctl(device->File, UI_SET_KEYBIT, BTN_X);
            ioctl(device->File, UI_SET_KEYBIT, BTN_Y);
            ioctl(device->File, UI_SET_KEYBIT, BTN_TL);
            ioctl(device->File, UI_SET_KEYBIT, BTN_TR);
            ioctl(device->File, UI_SET_KEYBIT, BTN_SELECT);
            ioctl(device->File, UI_SET_KEYBIT, BTN_START);
            // Axis.
            ioctl(device->File, UI_SET_EVBIT, EV_ABS);
            ioctl(device->File, UI_SET_ABSBIT, ABS_X);
            ioctl(device->File, UI_SET_ABSBIT, ABS_Y);
            userInput.absmin[ABS_X] = 0;
            userInput.absmax[ABS_X] = 4;
            userInput.absmin[ABS_Y] = 0;
            userInput.absmax[ABS_Y] = 4;
            break;
        case INPUT_KEYBOARD:
            for (unsigned int i = 0; i < TotalInputKeys; i++) {
                ioctl(device->File, UI_SET_KEYBIT, InputKeyValues[i]);
            }
            break;
    }

	// Add input device into input sub-system
	write(device->File, &userInput, sizeof(userInput));
	if (ioctl(device->File, UI_DEV_CREATE) < 0) {
		return false;
	}

    if(deviceType == INPUT_GAMEPAD){
        WriteAxis(device, ABS_X, 2);
        WriteAxis(device, ABS_Y, 2);
    }

	return true;
}

bool CloseInputDevice(InputDevice *const device)
{
	ioctl(device->File, UI_DEV_DESTROY);
	return close(device->File) == 0;
}

bool WriteAxis(InputDevice *const device, unsigned short int axis, DigitalAxisValue value) {
    struct input_event event;
    memset(&event, 0, sizeof(event));

    event.type = EV_ABS;
    event.code = axis;
    event.value = value;

    if (write(device->File, &event, sizeof(event)) <= 0) {
        fprintf(stderr, "Unable to write axis to '%s'\n", device->Name);
        return false;
    }

    return true;
}

bool WriteKey(InputDevice *const device, unsigned short int key, bool keyPressed) {
	struct input_event event;
    memset(&event, 0, sizeof(event));

	event.type = EV_KEY;
	event.code = key;
	event.value = keyPressed;

	if (write(device->File, &event, sizeof(event)) <= 0) {
        fprintf(stderr, "Unable to write key to '%s'\n", device->Name);
        return false;
	}

    return true;
}

bool WriteSync(InputDevice *const device) {
    struct input_event event;
    memset(&event, 0, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;

    if (write(device->File, &event, sizeof(event)) <= 0) {
        fprintf(stderr, "Unable to write sync to '%s'\n", device->Name);
        return false;
    }
    return true;
}
