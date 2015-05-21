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

#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include <signal.h>

#include "btn.h"
#include "gamepad.h"
#include "uinput.h"

#include "config.h"
#include "daemon.h"

#define true_str "true"
#define false_str "false"

#define MAX_GAMEPADS 2 /* number of game pads to poll */

#define CONFIG_FILE "/etc/gpio/snesdev.cfg"

bool running;
InputDevice keyboardDevice;
InputDevice *gamepadDevices;
SNESDevConfig config;


void SetupSignals();
void SignalHander(int signal);

/* checks, if a button on the pad is pressed and sends an event according the button state. */
static inline void processPadBtn(uint16_t buttons, uint16_t mask, unsigned short int key, InputDevice * uinp_gpad) {
	if ((buttons & mask) == mask) {
		WriteKey(uinp_gpad, key, true);
	} else {
        WriteKey(uinp_gpad, key, false);
	}
}

int main(int argc, char *argv[]) {

    if(!TryGetSNESDevConfig(CONFIG_FILE, argc, argv, MAX_GAMEPADS, &config)) {
        return EXIT_FAILURE;
    }

    printf("RunAsDaemon: %s, DebugEnabled: %s, PidFile: %s\n",
		   config.RunAsDaemon ? true_str : false_str, config.DebugEnabled ? true_str : false_str, config.PidFile);
    printf("ClockGpio: %u, LatchGpio: %u, GamepadPollFrequency: %u\n",
           config.ClockGpio, config.LatchGpio, config.GamepadPollFrequency);

    for(unsigned int i=0; i < config.NumberOfGamepads; i++) {
        GamepadConfig *gamepad = &config.Gamepads[i];
        printf("Gamepad %u, Enabled: %s, Type: %d, Gpio: %u\n", gamepad->Id, gamepad->Enabled ? true_str : false_str, gamepad->Type, gamepad->DataGpio);
    }

    printf("ButtonEnabled: %s, ButtonGpio: %u, ButtonPollFrequency: %u\n",
           config.ButtonEnabled ? true_str : false_str, config.ButtonGpio, config.ButtonPollFrequency);

	bcm2835_set_debug((uint8_t) config.DebugEnabled);

	if (!bcm2835_init()) {
		return 1;
	}

    if(config.RunAsDaemon) {
        StartDaemon(config.PidFile);
    }

    GPAD_ST gamepads[config.NumberOfGamepads];
    BTN_DEV_ST button;

    gamepadDevices = malloc(config.NumberOfGamepads * sizeof(*gamepadDevices));

	if (config.NumberOfGamepads > 0) {

        for(unsigned int i = 0; i < config.NumberOfGamepads; i++) {
            GamepadConfig *gamepad = &config.Gamepads[i];

            gamepads[i].pin_clock = config.ClockGpio;
            gamepads[i].pin_strobe = config.LatchGpio;
            gamepads[i].pin_data = gamepad->DataGpio;
            gamepads[i].type = gamepad->Type;

            if(gamepad->Enabled) {
                printf("[SNESDev-Rpi] Enabling game pad %d with type '%s'.\n",
                       gamepad->Id, GetGamepadTypeString(gamepad->Type));
                gpad_open(&gamepads[i]);
                OpenInputDevice(Gamepad, &gamepadDevices[i]);
            }
        }
	}

	if (config.ButtonEnabled) {
		printf("[SNESDev-Rpi] Enabling button.\n");
        button.pin = config.ButtonGpio;
		btn_open(&button);
        OpenInputDevice(Keyboard, &keyboardDevice);
	}

    SetupSignals();

	const unsigned int frameLength = config.NumberOfGamepads > 0 ? config.GamepadPollFrequency : config.ButtonPollFrequency;
    const unsigned int buttonFrameDelay = (frameLength + config.ButtonPollFrequency - 1) / config.ButtonPollFrequency;
    unsigned int frameDelayCount = 0;

	running = true;
	while (running) {
        for(unsigned int i = 0; i < config.NumberOfGamepads; i++) {
            GamepadConfig *gamepadConfig = &config.Gamepads[i];
            if(!gamepadConfig->Enabled) {
                continue;
            }

            // Read states of the buttons.
            ReadGamepads(&gamepads[i]);

            InputDevice *gamepadDevice = &gamepadDevices[i];
            const uint16_t state = gamepads[i].state;

            processPadBtn(state, GPAD_SNES_A, BTN_A, gamepadDevice);
            processPadBtn(state, GPAD_SNES_B, BTN_B, gamepadDevice);
            processPadBtn(state, GPAD_SNES_X, BTN_X, gamepadDevice);
            processPadBtn(state, GPAD_SNES_Y, BTN_Y, gamepadDevice);
            processPadBtn(state, GPAD_SNES_L, BTN_TL, gamepadDevice);
            processPadBtn(state, GPAD_SNES_R, BTN_TR, gamepadDevice);
            processPadBtn(state, GPAD_SNES_SELECT, BTN_SELECT, gamepadDevice);
            processPadBtn(state, GPAD_SNES_START, BTN_START, gamepadDevice);

            // X Axis.
            if ((state & GPAD_SNES_LEFT) == GPAD_SNES_LEFT) {
                WriteAxis(gamepadDevice, ABS_X, 0);
            } else if ((state & GPAD_SNES_RIGHT) == GPAD_SNES_RIGHT) {
                WriteAxis(gamepadDevice, ABS_X, 4);
            } else {
                WriteAxis(gamepadDevice, ABS_X, 2);
            }

            // Y Axis.
            if ((state & GPAD_SNES_UP) == GPAD_SNES_UP) {
                WriteAxis(gamepadDevice, ABS_Y, 0);
            } else if ((state & GPAD_SNES_DOWN) == GPAD_SNES_DOWN) {
                WriteAxis(gamepadDevice, ABS_Y, 4);
            } else {
                WriteAxis(gamepadDevice, ABS_Y, 2);
            }

            WriteSync(gamepadDevice);
        }

		if (config.ButtonEnabled && frameDelayCount == 0) {
			btn_read(&button);

			switch (button.state) {
			case BTN_STATE_IDLE:
				break;
			case BTN_STATE_PRESSED:
                WriteKey(&keyboardDevice, KEY_ESC, true);
                WriteSync(&keyboardDevice);
				break;
			case BTN_STATE_RELEASED:
                WriteKey(&keyboardDevice, KEY_ESC, false);
                WriteSync(&keyboardDevice);
				break;
			}
		}
        frameDelayCount++;
        frameDelayCount %= buttonFrameDelay;
        bcm2835_delay(frameLength);
	}

	return 0;
}


void SetupSignals() {
    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SignalHander);
    signal(SIGINT, SignalHander);
}

void SignalHander(int signal) {
    CloseInputDevice(&keyboardDevice);

    for (unsigned int i = 0; i < config.NumberOfGamepads; i++) {
        CloseInputDevice(&gamepadDevices[i]);
    }

    running = false;
}
