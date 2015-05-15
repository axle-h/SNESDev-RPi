cmake_minimum_required(VERSION 2.8.4)
project(SNESDev)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -ftemplate-backtrace-limit=0 -DSDL_MAIN_HANDLED")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=gnu11")


# Source files
file(GLOB_RECURSE SOURCE_FILES "src/*.h" "src/*.c")

find_package(Confuse REQUIRED)
find_package(BCM2835 REQUIRED)

include_directories(include
	${CONFUSE_INCLUDE_DIRS}
	${BCM2835_INCLUDE_DIR})

add_executable(SNESDev ${SOURCE_FILES})
target_link_libraries(SNESDev
	${CONFUSE_LIBRARIES}
	${BCM2835_LIBRARIES})

