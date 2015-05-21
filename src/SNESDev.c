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
InputDevice virtualKeyboard;
InputDevice *virtualGamepads;
SNESDevConfig config;


void SetupSignals();
void SignalHander(int signal);

/* checks, if a button on the pad is pressed and sends an event according the button state. */
static inline void processPadBtn(uint16_t buttons, uint16_t mask, uint16_t key, InputDevice * uinp_gpad) {
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

    GPAD_ST gpads[config.NumberOfGamepads];
    BTN_DEV_ST button;

    virtualGamepads = malloc(config.NumberOfGamepads * sizeof(*virtualGamepads));

	if (config.NumberOfGamepads > 0) {

        for(uint8_t i = 0; i < config.NumberOfGamepads; i++) {
            GamepadConfig *gamepad = &config.Gamepads[i];

            gpads[i].port = 1;
            gpads[i].pin_clock = config.ClockGpio;
            gpads[i].pin_strobe = config.LatchGpio;
            gpads[i].pin_data = gamepad->DataGpio;
            gpads[i].type = gamepad->Type;

            if(gamepad->Enabled) {
                printf("[SNESDev-Rpi] Enabling game pad %d with type '%s'.\n",
                       gamepad->Id, gamepad->Type == GPAD_TYPE_NES ? "NES" : "SNES");
                gpad_open(&gpads[i]);
                OpenInputDevice(Gamepad, &virtualGamepads[i]);
            }
        }
	}

	if (config.ButtonEnabled) {
		printf("[SNESDev-Rpi] Enabling button.\n");
		button.port = 1;
        button.pin = config.ButtonGpio;
		btn_open(&button);
        OpenInputDevice(Keyboard, &virtualKeyboard);
	}

    SetupSignals();

	uint32_t frameLength = config.NumberOfGamepads > 0 ? config.GamepadPollFrequency : config.ButtonPollFrequency;
    uint8_t buttonFrameDelay = (uint8_t) ((frameLength + config.ButtonPollFrequency - 1) / config.ButtonPollFrequency);
    uint8_t frameDelayCount = 0;

	running = true;
	while (running) {
        for(uint8_t i = 0; i < config.NumberOfGamepads; i++) {
            GamepadConfig *gamepad = &config.Gamepads[i];
            if(!gamepad->Enabled) {
                continue;
            }

            // Read states of the buttons.
            gpad_read(&gpads[i]);

            InputDevice *gpad = &virtualGamepads[i];
            const uint16_t state = gpads[i].state;

            processPadBtn(state, GPAD_SNES_A, BTN_A, gpad);
            processPadBtn(state, GPAD_SNES_B, BTN_B, gpad);
            processPadBtn(state, GPAD_SNES_X, BTN_X, gpad);
            processPadBtn(state, GPAD_SNES_Y, BTN_Y, gpad);
            processPadBtn(state, GPAD_SNES_L, BTN_TL, gpad);
            processPadBtn(state, GPAD_SNES_R, BTN_TR, gpad);
            processPadBtn(state, GPAD_SNES_SELECT, BTN_SELECT, gpad);
            processPadBtn(state, GPAD_SNES_START, BTN_START, gpad);

            // X Axis.
            if ((state & GPAD_SNES_LEFT) == GPAD_SNES_LEFT) {
                WriteAxis(gpad, ABS_X, 0);
            } else if ((state & GPAD_SNES_RIGHT) == GPAD_SNES_RIGHT) {
                WriteAxis(gpad, ABS_X, 4);
            } else {
                WriteAxis(gpad, ABS_X, 2);
            }

            // Y Axis.
            if ((state & GPAD_SNES_UP) == GPAD_SNES_UP) {
                WriteAxis(gpad, ABS_Y, 0);
            } else if ((state & GPAD_SNES_DOWN) == GPAD_SNES_DOWN) {
                WriteAxis(gpad, ABS_Y, 4);
            } else {
                WriteAxis(gpad, ABS_Y, 2);
            }
        }

		if (config.ButtonEnabled && frameDelayCount == 0) {
			btn_read(&button);

			switch (button.state) {
			case BTN_STATE_IDLE:
				break;
			case BTN_STATE_PRESSED:
                WriteKey(&virtualKeyboard, KEY_ESC, true);
				break;
			case BTN_STATE_RELEASED:
                WriteKey(&virtualKeyboard, KEY_ESC, false);
				break;
			}
		}

        frameDelayCount = ++frameDelayCount % buttonFrameDelay;
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
    CloseInputDevice(&virtualKeyboard);
    for (uint8_t i = 0; i < config.NumberOfGamepads; i++) {
        CloseInputDevice(&virtualGamepads[i]);
    }

    running = false;
}
