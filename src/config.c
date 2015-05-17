#include <stdbool.h>
#include <confuse.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "config.h"

#define CFG_DEBUG_ENABLED "DebugEnabled"
#define CFG_PID_FILE "PidFile"
#define CFG_CLOCK_GPIO "ClockGpio"
#define CFG_LATCH_GPIO "LatchGpio"

#define CFG_ENABLED "Enabled"
#define CFG_GPIO "Gpio"
#define CFG_POLL_FREQ "PollFrequency"

#define MAX_GAMEPADS 2
#define CFG_GAMEPADS "Gamepads"
#define CFG_GAMEPAD "Gamepad"
#define CFG_GAMEPAD_TYPE "Type"
#define CFG_BUTTON "Button"

int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);

bool TryGetSNESDevConfig(const char *fileName, SNESDevConfig *config) {
    cfg_opt_t GamepadOpts[] = {
            CFG_BOOL(CFG_ENABLED, cfg_false, CFGF_NONE),
            CFG_INT_CB(CFG_GAMEPAD_TYPE, 0, CFGF_NONE, &VerifyGamepadType),
            CFG_INT(CFG_GPIO, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t GamepadsOpts[] = {
            CFG_SEC(CFG_GAMEPAD, GamepadOpts, CFGF_MULTI | CFGF_TITLE),
            CFG_INT(CFG_CLOCK_GPIO, 0, CFGF_NONE),
            CFG_INT(CFG_LATCH_GPIO, 0, CFGF_NONE),
            CFG_INT(CFG_POLL_FREQ, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t ButtonOpts[] = {
            CFG_BOOL(CFG_ENABLED, cfg_false, CFGF_NONE),
            CFG_INT(CFG_GPIO, 0, CFGF_NONE),
            CFG_INT(CFG_POLL_FREQ, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t opts[] = {
            CFG_BOOL(CFG_DEBUG_ENABLED, cfg_false, CFGF_NONE),
            CFG_STR(CFG_PID_FILE, 0, CFGF_NONE),
            CFG_SEC(CFG_GAMEPADS, GamepadsOpts, CFGF_NONE),
            CFG_SEC(CFG_BUTTON, ButtonOpts, CFGF_NONE),
            CFG_END()
    };

    cfg_t *cfg;
    cfg = cfg_init(opts, CFGF_NOCASE);

    if (access(fileName, F_OK) == -1 || cfg_parse(cfg, fileName) != CFG_SUCCESS) {
        return false;
    }

    config->DebugEnabled = cfg_getbool(cfg, CFG_DEBUG_ENABLED);

    char *pidFile = cfg_getstr(cfg, CFG_PID_FILE);
    char *tmp = malloc(sizeof pidFile);
    strcpy(tmp, pidFile);
    config->PidFile = tmp;

    // Parse gamepad section
    cfg_t *gamepadsSection = cfg_getsec(cfg, CFG_GAMEPADS);
    config->ClockGpio = (uint8_t) cfg_getint(gamepadsSection, CFG_CLOCK_GPIO);
    config->LatchGpio = (uint8_t) cfg_getint(gamepadsSection, CFG_LATCH_GPIO);
    config->GamepadPollFrequency = (uint32_t) cfg_getint(gamepadsSection, CFG_POLL_FREQ);

    uint8_t n = (uint8_t) cfg_size(gamepadsSection, CFG_GAMEPAD);
    GamepadConfig gamepads[n];

    for(uint8_t i = 0; i < n; i++) {
        cfg_t *gamepadSection = cfg_getnsec(gamepadsSection, CFG_GAMEPAD, i);
        GamepadConfig gamepad;
        int id = atoi(cfg_title(gamepadSection));
        if(id <= 0) {
            cfg_error(cfg, "Must specify gamepad id");
            return false;
        }
        if(id > MAX_GAMEPADS) {
            cfg_error(cfg, "Only %d gamepads supported", MAX_GAMEPADS);
            return false;
        }

        gamepad.Id = (uint8_t) id;
        gamepad.Enabled = cfg_getbool(gamepadSection, CFG_ENABLED) ? true : false;
        gamepad.Type = (GPAD_TYPE)cfg_getint(gamepadSection, CFG_GAMEPAD_TYPE);
        gamepad.DataGpio = (uint8_t) cfg_getint(gamepadSection, CFG_GPIO);

        gamepads[i] = gamepad;
    }

    config->NumberOfGamepads = n;
    config->Gamepads = &gamepads[0];

    // Parse button options.
    cfg_t *buttonSection = cfg_getsec(cfg, CFG_BUTTON);
    config->ButtonEnabled = cfg_getbool(buttonSection, CFG_ENABLED);
    config->ButtonGpio = (uint8_t) cfg_getint(buttonSection, CFG_GPIO);
    config->ButtonPollFrequency = (uint32_t) cfg_getint(buttonSection, CFG_POLL_FREQ);

    cfg_free(cfg);
    return true;
}


int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    if(strcmp(value, "snes") == 0)
        *(long int *)result = GPAD_TYPE_SNES;
    else if(strcmp(value, "nes") == 0)
        *(long int *)result = GPAD_TYPE_NES;
    else {
        cfg_error(cfg, "Gamepad type must be snes or nes");
        return -1;
    }
    return 0;
}
