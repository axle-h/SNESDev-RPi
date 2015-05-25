find_path(CONFUSE_INCLUDE_DIR confuse.h
    PATHS
    "$ENV{CONFUSE}/include"
    /usr/local/include
    /usr/include)

find_library(CONFUSE_STATIC_LIBRARIES libconfuse.a
    PATHS
    "$ENV{CONFUSE}/lib"
    /usr/local/lib
    /usr/lib)

SET(CONFUSE_FOUND 0)
IF(CONFUSE_INCLUDE_DIR)
    IF(CONFUSE_STATIC_LIBRARIES)
        SET(CONFUSE_FOUND 1)
        MESSAGE(STATUS "Found Confuse")
    ENDIF()
ENDIF()

mark_as_advanced(CONFUSE_INCLUDE_DIRS CONFUSE_STATIC_LIBRARIES)