#include <stdbool.h>
#include <confuse.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include "config.h"

#define CFG_CLOCK_GPIO "ClockGpio"
#define CFG_LATCH_GPIO "LatchGpio"

#define CFG_ENABLED "Enabled"
#define CFG_GPIO "Gpio"
#define CFG_POLL_FREQ "PollFrequency"

#define CFG_GAMEPADS "Gamepads"
#define CFG_GAMEPAD "Gamepad"
#define CFG_GAMEPAD_TYPE "Type"
#define CFG_BUTTON "Button"

#define OPT_USAGE ""
#define OPT_HELP "(S)NES controller user-space driver for Raspberry Pi GPIO"
#define OPT_DEBUG -1
#define OPT_DAEMON 'd'
#define OPT_PIDFILE 'p'

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
} Arguments;

static const error_t ParseOption(int key, char *arg, struct argp_state *state);
static const struct argp_option options[] = {
        { "daemon", OPT_DAEMON, 0, 0, "Run as a daemon" },
        { "debug", OPT_DEBUG, 0, 0, "Run with debug options set in gpio library" },
        { "pidfile", OPT_PIDFILE, "FILE", 0, "Write PID to FILE" },
        { 0 }
};

static const bool ValidateConfig(SNESDevConfig *config, const int maxGamepads);
static const int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static const Arguments ParseArguments(int argc, char **argv);

bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, const int maxGamepads, SNESDevConfig *config) {
    const Arguments arguments = ParseArguments(argc, argv);

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
            CFG_SEC(CFG_GAMEPADS, GamepadsOpts, CFGF_NONE),
            CFG_SEC(CFG_BUTTON, ButtonOpts, CFGF_NONE),
            CFG_END()
    };

    cfg_t *cfg;
    cfg = cfg_init(opts, CFGF_NOCASE);

    if (access(fileName, F_OK) == -1 || cfg_parse(cfg, fileName) != CFG_SUCCESS) {
        return false;
    }

    config->RunAsDaemon = !arguments.DebugEnabled && arguments.RunAsDaemon;
    config->DebugEnabled = arguments.DebugEnabled;
    config->PidFile = arguments.PidFile;

    // Parse gamepad section
    cfg_t *gamepadsSection = cfg_getsec(cfg, CFG_GAMEPADS);
    config->ClockGpio = (uint8_t) cfg_getint(gamepadsSection, CFG_CLOCK_GPIO);
    config->LatchGpio = (uint8_t) cfg_getint(gamepadsSection, CFG_LATCH_GPIO);
    config->GamepadPollFrequency = (uint32_t) cfg_getint(gamepadsSection, CFG_POLL_FREQ);

    uint8_t n = (uint8_t) cfg_size(gamepadsSection, CFG_GAMEPAD);
    GamepadConfig gamepads[n];

    // TODO: Sort by gamepad id.
    for(uint8_t i = 0; i < n; i++) {
        cfg_t *gamepadSection = cfg_getnsec(gamepadsSection, CFG_GAMEPAD, i);
        GamepadConfig gamepad;
        gamepad.Id = (uint8_t) atoi(cfg_title(gamepadSection));
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

    if(!ValidateConfig(config, maxGamepads)) {
        return false;
    }

    return true;
}


static const bool ValidateConfig(SNESDevConfig *config, const int maxGamepads) {
    if(config->RunAsDaemon && config->PidFile == NULL) {
        fprintf(stderr, "PID file required when running as daemon\n");
        return false;
    }

    if(config->GamepadPollFrequency <= 0) {
        fprintf(stderr, "Gamepad %s Must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    if(config->ClockGpio == 0) {
        fprintf(stderr, "%s must be > 0\n", CFG_CLOCK_GPIO);
        return false;
    }

    if(config->LatchGpio == 0) {
        fprintf(stderr, "%s must be > 0\n", CFG_LATCH_GPIO);
        return false;
    }

    if(config->NumberOfGamepads == 0) {
        fprintf(stderr, "No gamepads configured\n");
        return false;
    }

    bool anyEnabled = false;
    for(int i = 0; i < config->NumberOfGamepads; i++) {
        GamepadConfig *gamepad = &config->Gamepads[i];
        if(gamepad == NULL || gamepad->DataGpio <= 0 || gamepad->Id < 1 || gamepad->Id > maxGamepads) {
            fprintf(stderr, "Bad gamepad config\n");
            return false;
        }
        if(gamepad->Enabled) {
            anyEnabled = true;
        }
    }

    if(!anyEnabled) {
        fprintf(stderr, "No gamepads configured\n");
        return false;
    }

    if(!config->ButtonEnabled) {
        return true;
    }

    if(config->ButtonGpio <= 0) {
        fprintf(stderr, "Button %s Must be > 0\n", CFG_GPIO);
        return false;
    }

    if(config->ButtonPollFrequency <= 0) {
        fprintf(stderr, "Button %s Must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    return true;
}

static const int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
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


static const Arguments ParseArguments(const int argc, char **argv) {
    Arguments arguments;
    arguments.RunAsDaemon = false;
    arguments.DebugEnabled = false;
    arguments.PidFile = NULL;

    const struct argp argp = { options, ParseOption, OPT_USAGE, OPT_HELP };
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    return arguments;
}

static const error_t ParseOption(int key, char *arg, struct argp_state *state) {
    Arguments *arguments = state->input;

    switch (key)
    {
        case OPT_DAEMON:
            arguments->RunAsDaemon = true;
            break;
        case OPT_PIDFILE:
            arguments->PidFile = arg;
            break;
        case OPT_DEBUG:
            arguments->DebugEnabled = true;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}