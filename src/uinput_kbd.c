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

#include "uinput_kbd.h"


#define UINPUT_DEVICE "/dev/uinput"
#define UINPUT_DEVICE_NAME "SNESDev Keyboard"

DEFINE_ENUM(VirtualKey, VIRTUAL_KEYS, size_t)

bool OpenVirtualKeyboard(VirtualKeyboard *const keyboard)
{
	keyboard->FileDescriptor = open(UINPUT_DEVICE, O_WRONLY | O_NDELAY);
	if (keyboard->FileDescriptor < 0) {
		fprintf(stderr, "Unable to open %s\n", UINPUT_DEVICE);
		return false;
	}

	struct uinput_user_dev device;
	memset(&device, 0, sizeof(device));
	strncpy(device.name, UINPUT_DEVICE_NAME, strlen(UINPUT_DEVICE_NAME));
	device.id.version = 4;
	device.id.bustype = BUS_USB;
	device.id.product = 1;
	device.id.vendor = 1;

	// Setup the uinput device as a keyboard with basic keys
	ioctl(keyboard->FileDescriptor, UI_SET_EVBIT, EV_KEY);
	ioctl(keyboard->FileDescriptor, UI_SET_EVBIT, EV_REL);

    for (int i = 0; i < TotalVirtualKeys; i++) {
		ioctl(keyboard->FileDescriptor, UI_SET_KEYBIT, VirtualKeyValues[i]);
	}

	// Add input device into input sub-system
	write(keyboard->FileDescriptor, &device, sizeof(device));
	if (ioctl(keyboard->FileDescriptor, UI_DEV_CREATE) < 0) {
		return false;
	}

	return true;
}

bool CloseVirtualKeyboard(VirtualKeyboard *const keyboard)
{
	ioctl(keyboard->FileDescriptor, UI_DEV_DESTROY);
	return close(keyboard->FileDescriptor) == 0;
}

bool WriteToVirtualKeyboard(VirtualKeyboard *const keyboard, unsigned short int key, bool keyPressed)
{
	struct input_event event;
    memset(&event, 0, sizeof(event));

	event.type = EV_KEY;
	event.code = key;
	event.value = keyPressed;

	if (write(keyboard->FileDescriptor, &event, sizeof(event)) <= 0) {
        fprintf(stderr, "Unable to write key to '%s'\n", UINPUT_DEVICE_NAME);
        return false;
	}

    // TODO: Check whether a type = EV_SYN, code = SYN_REPORT event is needed. Think it's a bit overkill.

    return true;
}
