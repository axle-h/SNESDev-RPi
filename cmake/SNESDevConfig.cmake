if(NOT DEFINED KEYBOARD_DEVICE_NAME)
    set(KEYBOARD_DEVICE_NAME "SNESDev Keyboard")
endif()

if(NOT DEFINED GAMEPAD_DEVICE_NAME)
    set(GAMEPAD_DEVICE_NAME "SNESDev Gamepad")
endif()

if(NOT DEFINED LOG_IDENTITY)
    set(LOG_IDENTITY "SNESDev")
endif()

if(NOT DEFINED CONFIG_FILE)
    set(CONFIG_FILE "/etc/gpio/snesdev.cfg")
endif()

if(NOT DEFINED SNESDEV_MAX_GAMEPADS)
    set(SNESDEV_MAX_GAMEPADS 2)
endif()

if(NOT DEFINED SNESDEV_MAX_BUTTONS)
    set(SNESDEV_MAX_BUTTONS 5)
endif()

configure_file(SNESDevConfig.h.in ${PROJECT_BINARY_DIR}/include/SNESDevConfig.h)
include_directories(include  ${PROJECT_BINARY_DIR}/include)