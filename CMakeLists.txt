cmake_minimum_required(VERSION 2.8.4)
project(SNESDev)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
configure_file (${PROJECT_SOURCE_DIR}/scripts/SNESDev.in ${PROJECT_BINARY_DIR}/scripts/SNESDevs)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=gnu11")

# Source files
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.c")

find_package(Confuse REQUIRED)
find_package(BCM2835 REQUIRED)

include_directories(include
	${CONFUSE_INCLUDE_DIRS}
	${BCM2835_INCLUDE_DIR})

add_executable(SNESDev ${SOURCE_FILES})
target_link_libraries(SNESDev
	${CONFUSE_LIBRARIES}
	${BCM2835_LIBRARIES})

install(TARGETS SNESDev DESTINATION sbin)
install(FILES "${PROJECT_BINARY_DIR}/scripts/SNESDev" DESTINATION /etc/init.d)
install(FILES "${PROJECT_SOURCE_DIR}/scripts/snesdev.cfg" DESTINATION /etc)
install(CODE "execute_process(COMMAND chmod +x /etc/init.d/SNESDev)")
install(CODE "execute_process(COMMAND systemctl daemon-reload)")
install(CODE "execute_process(COMMAND service SNESDev start)")
install(CODE "execute_process(COMMAND update-rc.d SNESDev defaults)")

