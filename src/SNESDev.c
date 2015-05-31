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

#include "config.h"
#include "daemon.h"


bool running;

void InitLog(SNESDevConfig *config);
void ConfigureGamepads(GamepadsConfig *config, Gamepad *gamepads, InputDevice *gamepadDevices);
void ConfigureButtons(ButtonsConfig *config, Button *buttons, InputDevice *keyboardDevice);
void ProcessGamepadFrame(GamepadsConfig *config, Gamepad *gamepads, InputDevice *gamepadDevices, bool verbose);
void ProcessButtonFrame(Button *buttons, InputDevice *keyboardDevice, unsigned int numberOfEnabledButtons, bool verbose);
void SetupSignals();
void SignalHandler(int signal);


int main(int argc, char *argv[]) {
    SNESDevConfig config;
    if(!TryGetSNESDevConfig(CONFIG_FILE, argc, argv, &config)) {
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

    Gamepad gamepads[config.Gamepads.Total];
    InputDevice gamepadDevices[config.Gamepads.Total];
    ConfigureGamepads(&config.Gamepads, &gamepads[0], &gamepadDevices[0]);

    Button buttons[config.Buttons.Total];
    InputDevice keyboardDevice;
    ConfigureButtons(&config.Buttons, &buttons[0], &keyboardDevice);

    SetupSignals();

    bool runButtonFrame = config.Buttons.Total > 0 && config.Buttons.PollFrequency > 0;
    const unsigned int buttonFrameDelay = runButtonFrame
                    ? (config.Gamepads.PollFrequency + config.Buttons.PollFrequency - 1) / config.Gamepads.PollFrequency
                    : 0;

    unsigned int frameDelayCount = 0;
	while (running) {
        ProcessGamepadFrame(&config.Gamepads, gamepads, gamepadDevices, config.Verbose);

        if(runButtonFrame) {
            if (config.Buttons.Total > 0 && frameDelayCount == 0) {
                ProcessButtonFrame(buttons, &keyboardDevice, config.Buttons.Total, config.Verbose);
            }

            frameDelayCount++;
            frameDelayCount %= buttonFrameDelay;
        }

        bcm2835_delay(config.Gamepads.PollFrequency);
	}

    CloseInputDevice(&keyboardDevice);
    for (unsigned int i = 0; i < config.Gamepads.Total; i++) {
        CloseInputDevice(&gamepadDevices[i]);
    }

    closelog();
    bcm2835_close();
	return 0;
}

void InitLog(SNESDevConfig *const config) {
    // Open the log file
    openlog(LOG_IDENTITY, LOG_PID, config->RunAsDaemon ? LOG_DAEMON : LOG_USER);

    for(unsigned int i = 0; i < config->Gamepads.Total; i++) {
        GamepadConfig *gamepad = config->Gamepads.Gamepads + i;

        syslog(LOG_INFO, "Gamepad%u: { Type: %s, PollFrequency: %u, Gpio: { Data: %u, Clock: %u, Latch: %u } }",
               gamepad->Id, GetGamepadTypeString(config->Gamepads.Type), config->Gamepads.PollFrequency,
               gamepad->DataGpio, config->Gamepads.ClockGpio, config->Gamepads.LatchGpio);
    }

    for(unsigned int i = 0; i < config->Buttons.Total; i++) {
        ButtonConfig *button = config->Buttons.Buttons + i;

        syslog(LOG_INFO, "Button%u: { Key: %s, PollFrequency: %u, Gpio: { Data: %u } }",
               button->Id, GetInputKeyString(button->Key), config->Buttons.PollFrequency, button->DataGpio);
    }
}

void ConfigureGamepads(GamepadsConfig *const config, Gamepad *const gamepads, InputDevice *const gamepadDevices) {
    for(unsigned int i = 0; i < config->Total; i++) {
        GamepadConfig *gamepadConfig = config->Gamepads + i;

        // Open uinput gamepad device.
        InputDevice *gamepadDevice = &gamepadDevices[i];
        char buffer[strlen(GAMEPAD_DEVICE_NAME) + 3];
        sprintf(buffer, "%s %u", GAMEPAD_DEVICE_NAME, gamepadConfig->Id);
        strcpy(&gamepadDevice->Name[0], buffer);

        OpenInputDevice(INPUT_GAMEPAD, gamepadDevice);

        // Open gamepad GPIO interface.
        Gamepad *gamepad = &gamepads[i];
        memset(gamepad, 0, sizeof(Gamepad));
        gamepad->DataGpio = gamepadConfig->DataGpio;
        OpenGamepad(gamepad);
    }

    OpenGamepadControlPins(config);
}

void ConfigureButtons(ButtonsConfig *const config, Button *const buttons, InputDevice *const keyboardDevice) {
    if(config->Total == 0) {
        return;
    }

    for(unsigned int i = 0; i < config->Total; i++) {
        ButtonConfig *buttonConfig = config->Buttons + i;

        // Open button GPIO interface.
        Button *button = &buttons[i];
        memset(button, 0, sizeof(Button));
        button->Gpio = buttonConfig->DataGpio;
        button->Key = buttonConfig->Key;
        OpenButton(button);
    }

    strcpy(keyboardDevice->Name, KEYBOARD_DEVICE_NAME);
    OpenInputDevice(INPUT_KEYBOARD, keyboardDevice);
}

void ProcessGamepadFrame(GamepadsConfig *const config, Gamepad *const gamepads, InputDevice *const gamepadDevices, bool verbose) {
    // Read states of the buttons.
    ReadGamepads(&gamepads[0], config);

    for(unsigned int i = 0; i < config->Total; i++) {
        Gamepad *gamepad = gamepads + i;

        if(!CheckGamepadState(gamepad)) {
            continue;
        }

        if(verbose) {
            printf("[%u] A: %d, B: %d, X: %d, Y: %d, L: %d, R: %d, Select: %d, Start: %d, XAxis: %u, YAxis: %u\n", i + 1,
                   gamepad->A, gamepad->B, gamepad->X, gamepad->Y, gamepad->L, gamepad->R, gamepad->Select, gamepad->Start,
                   gamepad->XAxis, gamepad->YAxis);
        }

        InputDevice *gamepadDevice = &gamepadDevices[i];
        WriteKey(gamepadDevice, BTN_A, gamepad->A);
        WriteKey(gamepadDevice, BTN_B, gamepad->B);
        WriteKey(gamepadDevice, BTN_X, gamepad->X);
        WriteKey(gamepadDevice, BTN_Y, gamepad->Y);
        WriteKey(gamepadDevice, BTN_TL, gamepad->L);
        WriteKey(gamepadDevice, BTN_TR, gamepad->R);
        WriteKey(gamepadDevice, BTN_SELECT, gamepad->Select);
        WriteKey(gamepadDevice, BTN_START, gamepad->Start);
        WriteAxis(gamepadDevice, ABS_X, gamepad->XAxis);
        WriteAxis(gamepadDevice, ABS_Y, gamepad->YAxis);
        WriteSync(gamepadDevice);
    }
}

void ProcessButtonFrame(Button *const buttons, InputDevice *const keyboardDevice, unsigned int numberOfEnabledButtons, bool verbose) {
    for(unsigned int i = 0; i < numberOfEnabledButtons; i++) {
        Button *button = buttons + i;

        ReadButton(button);

        switch (button->State) {
            case BUTTON_STATE_IDLE:
                break;
            case BUTTON_STATE_PRESSED:
                WriteKey(keyboardDevice, button->Key, true);
                WriteSync(keyboardDevice);
                if(verbose) {
                    printf("Button pressed on Gpio: %u, triggerred key: %s\n", button->Gpio, GetInputKeyString(button->Key));
                }
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
