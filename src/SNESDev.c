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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>
#include <signal.h>

#include "btn.h"
#include "gamepad.h"
#include "uinput_kbd.h"
#include "uinput_gamepad.h"

#include "config.h"

#define true_str "true"
#define false_str "false"

#define MAX_GAMEPADS 2 /* number of game pads to poll */

#define CONFIG_FILE "/etc/gpio/snesdev.cfg"

int16_t doRun, pollButton, pollPads;
UINP_KBD_DEV uinp_kbd;
UINP_GPAD_DEV uinp_gpads[MAX_GAMEPADS];

/* Signal callback function */
void sig_handler(int signo) {
	int16_t ctr = 0;

	if ((signo == SIGINT) | (signo == SIGQUIT) | (signo == SIGABRT) | (signo =
			SIGTERM)) {
		printf("Releasing SNESDev-Rpi device(s).\n");
		pollButton = 0;
		pollPads = 0;
		uinput_kbd_close(&uinp_kbd);
		for (ctr = 0; ctr < MAX_GAMEPADS; ctr++) {
			uinput_gpad_close(&uinp_gpads[ctr]);
		}

		doRun = 0;
	}
}

void register_signalhandlers() {
	/* Register signal handlers  */
	if (signal(SIGINT, sig_handler) == SIG_ERR )
		printf("\n[SNESDev-Rpi] Cannot catch SIGINT\n");
	if (signal(SIGQUIT, sig_handler) == SIG_ERR )
		printf("\n[SNESDev-Rpi] Cannot catch SIGQUIT\n");
	if (signal(SIGABRT, sig_handler) == SIG_ERR )
		printf("\n[SNESDev-Rpi] Cannot catch SIGABRT\n");
	if (signal(SIGTERM, sig_handler) == SIG_ERR )
		printf("\n[SNESDev-Rpi] Cannot catch SIGTERM\n");
}

/* checks, if a button on the pad is pressed and sends an event according the button state. */
static inline void processPadBtn(uint16_t buttons, uint16_t evtype, uint16_t mask, uint16_t key, UINP_GPAD_DEV* uinp_gpad) {
	if ((buttons & mask) == mask) {
		uinput_gpad_write(uinp_gpad, key, 1, evtype);
	} else {
		uinput_gpad_write(uinp_gpad, key, 0, evtype);
	}
}

int main(int argc, char *argv[]) {

    SNESDevConfig config;
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

    GPAD_ST gpads[config.NumberOfGamepads];
    BTN_DEV_ST button;

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
                uinput_gpad_open(&uinp_gpads[0]);
            }
        }
	}

	if (config.ButtonEnabled) {
		printf("[SNESDev-Rpi] Enabling button.\n");
		button.port = 1;
        button.pin = config.ButtonGpio;
		btn_open(&button);
		uinput_kbd_open(&uinp_kbd);
	}

	register_signalhandlers();

	uint32_t frameLength = config.NumberOfGamepads > 0 ? config.GamepadPollFrequency : config.ButtonPollFrequency;
    uint32_t frameCount = 0;

	doRun = 1;
	while (doRun) {
        for(uint8_t i = 0; i < config.NumberOfGamepads; i++) {
            GamepadConfig *gamepad = &config.Gamepads[i];
            if(!gamepad->Enabled) {
                continue;
            }

            // Read states of the buttons.
            gpad_read(&gpads[i]);

            UINP_GPAD_DEV *gpad = &uinp_gpads[i];
            const uint16_t state = gpads[i].state;

            processPadBtn(state, EV_KEY, GPAD_SNES_A, BTN_A, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_B, BTN_B, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_X, BTN_X, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_Y, BTN_Y, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_L, BTN_TL, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_R, BTN_TR, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_SELECT, BTN_SELECT, gpad);
            processPadBtn(state, EV_KEY, GPAD_SNES_START, BTN_START, gpad);

            // X Axis.
            if ((state & GPAD_SNES_LEFT) == GPAD_SNES_LEFT) {
                uinput_gpad_write(gpad, ABS_X, 0, EV_ABS);
            } else if ((state & GPAD_SNES_RIGHT) == GPAD_SNES_RIGHT) {
                uinput_gpad_write(gpad, ABS_X, 4, EV_ABS);
            } else {
                uinput_gpad_write(gpad, ABS_X, 2, EV_ABS);
            }

            // Y Axis.
            if ((state & GPAD_SNES_UP) == GPAD_SNES_UP) {
                uinput_gpad_write(gpad, ABS_Y, 0, EV_ABS);
            } else if ((state & GPAD_SNES_DOWN) == GPAD_SNES_DOWN) {
                uinput_gpad_write(gpad, ABS_Y, 4, EV_ABS);
            } else {
                uinput_gpad_write(gpad, ABS_Y, 2, EV_ABS);
            }
        }

		if (config.ButtonEnabled && frameCount == 0) {
			btn_read(&button);

			switch (button.state) {
			case BTN_STATE_IDLE:
				break;
			case BTN_STATE_PRESSED:
				uinput_kbd_write(&uinp_kbd, KEY_ESC, 1, EV_KEY);
				break;
			case BTN_STATE_RELEASED:
                uinput_kbd_write(&uinp_kbd, KEY_ESC, 0, EV_KEY);
				break;
			}
		}

        frameCount = (frameCount + frameLength) % config.ButtonPollFrequency;
        bcm2835_delay(frameLength);
	}

	return 0;

}
