#include <stdbool.h>
#include <confuse.h>
#include <unistd.h>
#include <stdlib.h>
#include <argp.h>
#include "config.h"

#define CFG_CLOCK_GPIO "ClockGpio"
#define CFG_LATCH_GPIO "LatchGpio"

#define CFG_ENABLED "Enabled"
#define CFG_GPIO "Gpio"
#define CFG_KEY "Key"
#define CFG_POLL_FREQ "PollFrequency"

#define CFG_GAMEPADS "Gamepads"
#define CFG_GAMEPAD "Gamepad"
#define CFG_GAMEPAD_TYPE "Type"

#define CFG_BUTTONS "Buttons"
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

static error_t ParseOption(int key, char *arg, struct argp_state *state);
static const struct argp_option options[] = {
        { "daemon", OPT_DAEMON, 0, 0, "Run as a daemon", 0 },
        { "debug", OPT_DEBUG, 0, 0, "Run with debug options set in gpio library", 0 },
        { "pidfile", OPT_PIDFILE, "FILE", 0, "Write PID to FILE", 0 },
        { 0 }
};

static bool ValidateConfig(SNESDevConfig *config, const unsigned int maxGamepads);
static int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static int VerifyInputKey(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static Arguments ParseArguments(int argc, char **argv);

bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, const unsigned int maxGamepads, SNESDevConfig *config) {
    const Arguments arguments = ParseArguments(argc, argv);

    cfg_opt_t GamepadOpts[] = {
            CFG_BOOL(CFG_ENABLED, cfg_false, CFGF_NONE),
            CFG_INT(CFG_GPIO, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t GamepadsOpts[] = {
            CFG_SEC(CFG_GAMEPAD, GamepadOpts, CFGF_MULTI | CFGF_TITLE),
            CFG_INT_CB(CFG_GAMEPAD_TYPE, 0, CFGF_NONE, &VerifyGamepadType),
            CFG_INT(CFG_CLOCK_GPIO, 0, CFGF_NONE),
            CFG_INT(CFG_LATCH_GPIO, 0, CFGF_NONE),
            CFG_INT(CFG_POLL_FREQ, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t ButtonOpts[] = {
            CFG_BOOL(CFG_ENABLED, cfg_false, CFGF_NONE),
            CFG_INT_CB(CFG_KEY, 0, CFGF_NONE, &VerifyInputKey),
            CFG_INT(CFG_GPIO, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t ButtonsOpts[] = {
            CFG_SEC(CFG_BUTTON, ButtonOpts, CFGF_MULTI | CFGF_TITLE),
            CFG_INT(CFG_POLL_FREQ, 0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t opts[] = {
            CFG_SEC(CFG_GAMEPADS, GamepadsOpts, CFGF_NONE),
            CFG_SEC(CFG_BUTTONS, ButtonsOpts, CFGF_NONE),
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
    config->GamepadPollFrequency = (unsigned int) cfg_getint(gamepadsSection, CFG_POLL_FREQ);
    config->Type = (GamepadType)cfg_getint(gamepadsSection, CFG_GAMEPAD_TYPE);
    config->NumberOfGamepads = cfg_size(gamepadsSection, CFG_GAMEPAD);

    if(config->NumberOfGamepads != 0) {
        if(config->NumberOfGamepads > maxGamepads) {
            config->NumberOfGamepads = maxGamepads;
        }
        config->Gamepads = malloc(config->NumberOfGamepads * sizeof(GamepadConfig));

        // TODO: Sort by gamepad id.
        for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
            cfg_t *gamepadSection = cfg_getnsec(gamepadsSection, CFG_GAMEPAD, i);

            GamepadConfig *gamepadConfig = &config->Gamepads[i];
            gamepadConfig->Id = (unsigned int) atoi(cfg_title(gamepadSection));
            gamepadConfig->Enabled = cfg_getbool(gamepadSection, CFG_ENABLED) ? true : false;
            gamepadConfig->DataGpio = (uint8_t) cfg_getint(gamepadSection, CFG_GPIO);
        }
    }

    // Parse buttons section.
    cfg_t *buttonsSection = cfg_getsec(cfg, CFG_BUTTONS);
    config->NumberOfButtons =cfg_size(buttonsSection, CFG_BUTTON);

    if(config->NumberOfButtons != 0) {
        config->Buttons = malloc(config->NumberOfButtons * sizeof(ButtonConfig));

        // TODO: Sort by button id.
        for (unsigned int i = 0; i < config->NumberOfButtons; i++) {
            cfg_t *buttonSection = cfg_getnsec(buttonsSection, CFG_BUTTON, i);
            ButtonConfig *buttonConfig = &config->Buttons[i];

            buttonConfig->Id = (unsigned int) atoi(cfg_title(buttonSection));
            buttonConfig->Enabled = cfg_getbool(buttonSection, CFG_ENABLED) ? true : false;
            buttonConfig->Key = (InputKey) cfg_getint(buttonSection, CFG_KEY);
            buttonConfig->DataGpio = (uint8_t) cfg_getint(buttonSection, CFG_GPIO);
        }

        config->ButtonPollFrequency = (unsigned int) cfg_getint(buttonsSection, CFG_POLL_FREQ);
    }

    cfg_free(cfg);

    if(!ValidateConfig(config, maxGamepads)) {
        return false;
    }

    return true;
}


static bool ValidateConfig(SNESDevConfig *config, const unsigned int maxGamepads) {
    if(config->RunAsDaemon && config->PidFile == NULL) {
        fprintf(stderr, "PID file required when running as daemon\n");
        return false;
    }

    if(config->GamepadPollFrequency == 0) {
        fprintf(stderr, "Gamepad %s must be > 0\n", CFG_POLL_FREQ);
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
    for(unsigned int i = 0; i < config->NumberOfGamepads; i++) {
        GamepadConfig *gamepad = &config->Gamepads[i];
        if(gamepad == NULL || gamepad->DataGpio == 0 || gamepad->Id == 0 || gamepad->Id > maxGamepads) {
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

    anyEnabled = false;
    for(unsigned int i = 0; i < config->NumberOfButtons; i++) {
        ButtonConfig *button = &config->Buttons[i];
        if(button == NULL || button->DataGpio == 0 || button->Id == 0) {
            fprintf(stderr, "Bad button config\n");
            return false;
        }
        if(button->Enabled) {
            anyEnabled = true;
        }
    }

    if(!anyEnabled) {
        return true;
    }

    if(config->ButtonPollFrequency == 0) {
        fprintf(stderr, "Button %s must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    return true;
}

static int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    GamepadType type = GetGamepadTypeValue(value);
    if(type == 0) {
        cfg_error(cfg, "Gamepad type must be snes or nes");
        return -1;
    }
    *(long int *)result = type;

    return 0;
}

static int VerifyInputKey(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    InputKey key = GetInputKeyValue(value);
    if(key == 0) {
        cfg_error(cfg, "Key is not valid");
        return -1;
    }
    *(long int *)result = key;

    return 0;
}

static Arguments ParseArguments(const int argc, char **argv) {
    Arguments arguments;
    arguments.RunAsDaemon = false;
    arguments.DebugEnabled = false;
    arguments.PidFile = NULL;

    const struct argp argumentOptions = { options, ParseOption, OPT_USAGE, OPT_HELP, 0, 0, 0 };
    argp_parse (&argumentOptions, argc, argv, 0, 0, &arguments);

    return arguments;
}

static error_t ParseOption(int key, char *arg, struct argp_state *state) {
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