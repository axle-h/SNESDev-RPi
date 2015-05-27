
set(SNESDEV_MAX_GAMEPADS 2)
set(SNESDEV_MAX_BUTTONS 5)

configure_file(CMakeConfig.h.in ${PROJECT_BINARY_DIR}/include/CMakeConfig.h)
include_directories(include  ${PROJECT_BINARY_DIR}/include)