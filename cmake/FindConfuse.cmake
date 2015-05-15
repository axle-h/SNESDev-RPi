set(CONFUSE_DEFINITIONS "")

find_path(CONFUSE_INCLUDE_DIR confuse.h
          PATHS
          /usr/local/include
          /usr/include)

find_library(CONFUSE_LIBRARY NAMES confuse
             PATHS
             /usr/local/lib
             /usr/lib)

set(CONFUSE_LIBRARIES ${CONFUSE_LIBRARY})
set(CONFUSE_INCLUDE_DIRS ${CONFUSE_INCLUDE_DIR})

mark_as_advanced(CONFUSE_INCLUDE_DIRS CONFUSE_LIBRARIES)