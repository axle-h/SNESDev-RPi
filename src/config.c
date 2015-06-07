#include <stdbool.h>
#include <confuse.h>
#include <unistd.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include "config.h"

// Arguments
#define OPT_USAGE ""
#define OPT_HELP "(S)NES controller user-space driver for Raspberry Pi GPIO"
#define OPT_DEBUG -1
#define OPT_VERBOSE 'v'
#define OPT_DAEMON 'd'
#define OPT_PIDFILE 'p'

typedef struct {
    unsigned int Verbose;
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
} Arguments;

static const struct argp_option options[] = {
        { "verbose", OPT_VERBOSE, 0, 0, "Print more stuff", 0 },
        { "daemon", OPT_DAEMON, 0, 0, "Run as a daemon", 0 },
        { "debug", OPT_DEBUG, 0, 0, "Run with debug options set in gpio library", 0 },
        { "pidfile", OPT_PIDFILE, "FILE", 0, "Write PID to FILE", 0 },
        { 0 }
};

// Config file.
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

static Arguments ParseArguments(int argc, char **argv);
static error_t ParseOption(int key, char *arg, struct argp_state *state);
static bool ValidateConfig(SNESDevConfig *config);
static int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static int VerifyInputKey(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static inline unsigned int SafeToUnsigned(long x);

bool TryGetSNESDevConfig(const char *fileName, const int argc, char **argv, SNESDevConfig *const config) {
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
        fprintf(stderr, "Cannot read config file %s\n", fileName);
        return false;
    }

    // Initialize from arguments.
    memset(config, 0, sizeof(SNESDevConfig));
    config->RunAsDaemon = !arguments.DebugEnabled && arguments.RunAsDaemon;
    config->DebugEnabled = arguments.DebugEnabled;
    config->Verbose = config->RunAsDaemon ? 0 : arguments.Verbose;

    // PidFile came from argv so will be way down teh stack :-)
    config->PidFile = arguments.PidFile;

    // Parse gamepad section
    GamepadsConfig *gamepadsConfig = &config->Gamepads;
    cfg_t *gamepadsSection = cfg_getsec(cfg, CFG_GAMEPADS);
    gamepadsConfig->ClockGpio = (uint8_t) SafeToUnsigned(cfg_getint(gamepadsSection, CFG_CLOCK_GPIO));
    gamepadsConfig->LatchGpio = (uint8_t) SafeToUnsigned(cfg_getint(gamepadsSection, CFG_LATCH_GPIO));

    unsigned int pollFrequency = SafeToUnsigned(cfg_getint(gamepadsSection, CFG_POLL_FREQ));
    if(pollFrequency > 0) {
        gamepadsConfig->PollFrequency = (unsigned int)(1000 / (double)pollFrequency);
    }
    gamepadsConfig->Type = (GamepadType)cfg_getint(gamepadsSection, CFG_GAMEPAD_TYPE);
    unsigned int numberOfGamepads = cfg_size(gamepadsSection, CFG_GAMEPAD);

    // Parse gamepads
    // TODO: Sort by gamepad id.
    for(unsigned int i = 0; i < numberOfGamepads; i++) {
        cfg_t *gamepadSection = cfg_getnsec(gamepadsSection, CFG_GAMEPAD, i);

        bool enabled = cfg_getbool(gamepadSection, CFG_ENABLED) ? true : false;
        if(!enabled) {
            continue;
        }

        GamepadConfig *gamepadConfig = gamepadsConfig->Gamepads + gamepadsConfig->Total;
        gamepadConfig->Id = (unsigned int) atoi(cfg_title(gamepadSection));
        gamepadConfig->DataGpio = (uint8_t) SafeToUnsigned(cfg_getint(gamepadSection, CFG_GPIO));
        gamepadsConfig->Total++;

        if(gamepadsConfig->Total > SNESDEV_MAX_GAMEPADS) {
            break;
        }
    }

    // Parse buttons section.
    ButtonsConfig *buttonsConfig = &config->Buttons;
    cfg_t *buttonsSection = cfg_getsec(cfg, CFG_BUTTONS);
    pollFrequency = SafeToUnsigned(cfg_getint(buttonsSection, CFG_POLL_FREQ));
    if(pollFrequency > 0) {
        buttonsConfig->PollFrequency = (unsigned int) (1000 / (double)pollFrequency);
    }
    unsigned int numberOfButtons = cfg_size(buttonsSection, CFG_BUTTON);

    // Parse buttons
    // TODO: Sort by button id.
    for (unsigned int i = 0; i < numberOfButtons; i++) {
        cfg_t *buttonSection = cfg_getnsec(buttonsSection, CFG_BUTTON, i);

        bool enabled = cfg_getbool(buttonSection, CFG_ENABLED) ? true : false;
        if(!enabled) {
            continue;
        }

        ButtonConfig *buttonConfig = buttonsConfig->Buttons + buttonsConfig->Total;
        buttonConfig->Id = (unsigned int) atoi(cfg_title(buttonSection));
        buttonConfig->Key = (InputKey) cfg_getint(buttonSection, CFG_KEY);
        buttonConfig->DataGpio = (uint8_t) SafeToUnsigned(cfg_getint(buttonSection, CFG_GPIO));
        buttonsConfig->Total++;

        if(buttonsConfig->Total > SNESDEV_MAX_BUTTONS) {
            break;
        }
    }

    cfg_free(cfg);

    if(!ValidateConfig(config)) {
        return false;
    }

    return true;
}


static bool ValidateConfig(SNESDevConfig *const config) {
    if(config->RunAsDaemon && config->PidFile == NULL) {
        fprintf(stderr, "PID file required when running as daemon\n");
        return false;
    }

    if(config->Gamepads.PollFrequency == 0) {
        fprintf(stderr, "Gamepad %s must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    if(config->Gamepads.ClockGpio == 0) {
        fprintf(stderr, "%s must be > 0\n", CFG_CLOCK_GPIO);
        return false;
    }

    if(config->Gamepads.LatchGpio == 0) {
        fprintf(stderr, "%s must be > 0\n", CFG_LATCH_GPIO);
        return false;
    }

    if(config->Gamepads.Total == 0) {
        fprintf(stderr, "No gamepads configured\n");
        return false;
    }

    for(unsigned int i = 0; i < config->Gamepads.Total; i++) {
        GamepadConfig *gamepad = config->Gamepads.Gamepads + i;
        if(gamepad == NULL || gamepad->DataGpio == 0 || gamepad->Id == 0 || gamepad->Id > SNESDEV_MAX_GAMEPADS) {
            fprintf(stderr, "Bad gamepad config\n");
            return false;
        }
    }

    if(config->Buttons.Total == 0) {
        return true;
    }

    for(unsigned int i = 0; i < config->Buttons.Total; i++) {
        ButtonConfig *button = config->Buttons.Buttons + i;
        if(button == NULL || button->DataGpio == 0 || button->Id == 0 || button->Key <= 0) {
            fprintf(stderr, "Bad button config\n");
            return false;
        }
    }

    if(config->Buttons.PollFrequency == 0) {
        fprintf(stderr, "Button %s must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    return true;
}

static int VerifyGamepadType(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
    GamepadType type = GetGamepadTypeValue(value);
    if(type == 0) {
        cfg_error(cfg, "Gamepad type must be snes or nes");
        return -1;
    }
    *(long int *)result = type;

    return 0;
}

static int VerifyInputKey(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
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
    arguments.Verbose = 0;
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
        case OPT_VERBOSE:
            arguments->Verbose++;
            break;
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

static inline unsigned int SafeToUnsigned(long x) {
    return x < 0 ? 0 : (unsigned int)x;
}