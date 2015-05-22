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
#include <string.h>
#include <sys/syslog.h>

#include "button.h"
#include "gamepad.h"
#include "uinput.h"

#include "config.h"
#include "daemon.h"

#define true_str "true"
#define false_str "false"

#define MAX_GAMEPADS 2
#define KEYBOARD_DEVICE_NAME "SNESDev Keyboard"
#define GAMEPAD_DEVICE_NAME "SNESDev Gamepad"

#define LOG_IDENTITY "SNESDev"
#define CONFIG_FILE "/etc/gpio/snesdev.cfg"

bool running;

void InitLog(SNESDevConfig const* config);
void ConfigureGamepads(SNESDevConfig const* config, Gamepad *gamepads, InputDevice *gamepadDevices, GamepadControlPins *gamepadControlPins);
unsigned int ConfigureButtons(SNESDevConfig const* config, Button *buttons, InputDevice *keyboardDevice);
void ProcessGamepadFrame(Gamepad *gamepads, InputDevice *gamepadDevices, GamepadControlPins *gamepadControlPins);
void ProcessButtonFrame(Button *buttons, InputDevice *keyboardDevice, unsigned int numberOfEnabledButtons);
void SetupSignals();
void SignalHandler(int signal);
static inline void ProcessGamepadButton(InputDevice *gamepad, uint16_t state, uint16_t mask, unsigned short int key);

int main(int argc, char *argv[]) {
    SNESDevConfig config;
    if(!TryGetSNESDevConfig(CONFIG_FILE, argc, argv, MAX_GAMEPADS, &config)) {
        return EXIT_FAILURE;
    }

    InitLog(&config);

    bcm2835_set_debug((uint8_t) config.DebugEnabled);

	if (!bcm2835_init()) {
		return EXIT_FAILURE;
	}

    if(config.RunAsDaemon) {
        StartDaemon(config.PidFile);
    }

    GamepadControlPins gamepadControlPins;
    Gamepad gamepads[config.NumberOfGamepads];
    InputDevice gamepadDevices[config.NumberOfGamepads];
    ConfigureGamepads(&config, &gamepads[0], &gamepadDevices[0], &gamepadControlPins);

    Button buttons[config.NumberOfButtons];
    InputDevice keyboardDevice;
    unsigned int numberOfEnabledButtons = ConfigureButtons(&config, &buttons[0], &keyboardDevice);

    SetupSignals();

	const unsigned int frameLength = gamepadControlPins.NumberOfGamepads > 0 ? config.GamepadPollFrequency : config.ButtonPollFrequency;
    const unsigned int buttonFrameDelay = (frameLength + config.ButtonPollFrequency - 1) / config.ButtonPollFrequency;
    unsigned int frameDelayCount = 0;

	while (running) {
        ProcessGamepadFrame(gamepads, gamepadDevices, &gamepadControlPins);

        if (frameDelayCount == 0) {
            ProcessButtonFrame(buttons, &keyboardDevice, numberOfEnabledButtons);
        }

        frameDelayCount++;
        frameDelayCount %= buttonFrameDelay;
        bcm2835_delay(frameLength);
	}

    CloseInputDevice(&keyboardDevice);
    for (unsigned int i = 0; i < gamepadControlPins.NumberOfGamepads; i++) {
        CloseInputDevice(&gamepadDevices[i]);
    }

    free(config.Gamepads);
    free(config.Buttons);

    closelog();
    bcm2835_close();
	return 0;
}

void InitLog(SNESDevConfig const* config) {
    // Open the log file
    openlog(LOG_IDENTITY, LOG_PID, config->RunAsDaemon ? LOG_DAEMON : LOG_USER);

    for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
        GamepadConfig *gamepad = &config->Gamepads[i];

        if(!gamepad->Enabled) {
            continue;
        }
        syslog(LOG_INFO, "Gamepad%u: { Type: %s, PollFrequency: %u, Gpio: { Data: %u, Clock: %u, Latch: %u } }",
               gamepad->Id, GetGamepadTypeString(config->Type), config->GamepadPollFrequency, gamepad->DataGpio, config->ClockGpio, config->LatchGpio);
    }

    for(unsigned int i = 0; i < config->NumberOfButtons; i++) {
        ButtonConfig *button = &config->Buttons[i];

        if(!button->Enabled) {
            continue;
        }
        syslog(LOG_INFO, "Button%u: { Key: %s, PollFrequency: %u, Gpio: { Data: %u } }",
               button->Id, GetInputKeyString(button->Key), config->ButtonPollFrequency, button->DataGpio);
    }
}

void ConfigureGamepads(SNESDevConfig const* config, Gamepad *gamepads, InputDevice *gamepadDevices, GamepadControlPins *gamepadControlPins) {
    unsigned int numberOfEnabledGamepads = 0;

    for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
        GamepadConfig *gamepadConfig = &config->Gamepads[i];
        if(!gamepadConfig->Enabled) {
            continue;
        }

        // Open uinput gamepad device.
        InputDevice *gamepadDevice = &gamepadDevices[numberOfEnabledGamepads];
        char buffer[strlen(GAMEPAD_DEVICE_NAME) + 3];
        sprintf(buffer, "%s %u", GAMEPAD_DEVICE_NAME, gamepadConfig->Id);
        gamepadDevice->Name = strdup(buffer);
        OpenInputDevice(INPUT_GAMEPAD, gamepadDevice);

        // Open gamepad GPIO interface.
        Gamepad *gamepad = &gamepads[numberOfEnabledGamepads];
        gamepad->DataGpio = gamepadConfig->DataGpio;
        OpenGamepad(gamepad);

        numberOfEnabledGamepads++;
    }

    gamepadControlPins->ClockGpio = config->ClockGpio;
    gamepadControlPins->LatchGpio = config->LatchGpio;
    gamepadControlPins->NumberOfGamepads = numberOfEnabledGamepads;
    gamepadControlPins->Type = config->Type;

    if(numberOfEnabledGamepads > 0) {
        OpenGamepadControlPins(gamepadControlPins);
    }
}

unsigned int ConfigureButtons(SNESDevConfig const* config, Button *buttons, InputDevice *keyboardDevice) {
    unsigned int numberOfEnabledButtons = 0;
    for(unsigned int i = 0; i < config->NumberOfButtons; i++) {
        ButtonConfig *buttonConfig = &config->Buttons[i];
        if(!buttonConfig->Enabled) {
            continue;
        }

        // Open button GPIO interface.
        Button *button = &buttons[numberOfEnabledButtons];
        button->Gpio = buttonConfig->DataGpio;
        button->Key = buttonConfig->Key;
        OpenButton(button);
    }
    if(numberOfEnabledButtons > 0) {
        keyboardDevice->Name = KEYBOARD_DEVICE_NAME;
        OpenInputDevice(INPUT_KEYBOARD, keyboardDevice);
    }

    return numberOfEnabledButtons;
}

void ProcessGamepadFrame(Gamepad *gamepads, InputDevice *gamepadDevices, GamepadControlPins *gamepadControlPins) {
    // Read states of the buttons.
    ReadGamepads(&gamepads[0], gamepadControlPins);

    for(unsigned int i = 0; i < gamepadControlPins->NumberOfGamepads; i++) {
        InputDevice *gamepadDevice = &gamepadDevices[i];
        const uint16_t state = gamepads[i].State;

        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_A, BTN_A);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_B, BTN_B);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_X, BTN_X);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_Y, BTN_Y);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_L, BTN_TL);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_R, BTN_TR);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_SELECT, BTN_SELECT);
        ProcessGamepadButton(gamepadDevice, state, GPAD_SNES_START, BTN_START);

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
}

static inline void ProcessGamepadButton(InputDevice *gamepad, uint16_t state, uint16_t mask, unsigned short int key) {
    if ((state & mask) == mask) {
        WriteKey(gamepad, key, true);
    } else {
        WriteKey(gamepad, key, false);
    }
}

void ProcessButtonFrame(Button *buttons, InputDevice *keyboardDevice, unsigned int numberOfEnabledButtons) {
    for(unsigned int i = 0; i < numberOfEnabledButtons; i++) {
        Button *button = &buttons[i];

        ReadButton(button);

        switch (button->State) {
            case BUTTON_STATE_IDLE:
                break;
            case BUTTON_STATE_PRESSED:
                WriteKey(keyboardDevice, button->Key, true);
                WriteSync(keyboardDevice);
                break;
            case BUTTON_STATE_RELEASED:
                WriteKey(keyboardDevice, button->Key, false);
                WriteSync(keyboardDevice);
                break;
        }
    }
}

void SetupSignals() {
    running = true;

    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
}

void SignalHandler(int signal) {
    running = false;
}
