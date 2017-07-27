# - Try to find CMocka
# Once done this will define
#
#  CMOCKA_ROOT_DIR - Set this variable to the root installation of CMocka
#
# Read-Only variables:
#  CMOCKA_FOUND - system has CMocka
#  CMOCKA_INCLUDE_DIR - the CMocka include directory
#  CMOCKA_LIBRARIES - Link these to use CMocka
#  CMOCKA_DEFINITIONS - Compiler switches required for using CMocka

set(_CMOCKA_ROOT_HINTS
         /usr/local/lib/
         /usr/lib/
)

set(_CMOCKA_ROOT_PATHS
    "$ENV{PROGRAMFILES}/cmocka"
)

find_path(CMOCKA_ROOT_DIR
    NAMES
        include/cmocka.h
    HINTS
        ${_CMOCKA_ROOT_HINTS}
    PATHS
        ${_CMOCKA_ROOT_PATHS}
)

find_path(CMOCKA_INCLUDE_DIR
    NAMES
        cmocka.h
    PATHS
        ${CMOCKA_ROOT_DIR}/include
)

find_library(CMOCKA_LIBRARY
    NAMES
        cmocka
    PATHS
        ${CMOCKA_ROOT_DIR}/lib
         /usr/lib/i386-linux-gnu
)

if (CMOCKA_LIBRARY)
  set(CMOCKA_LIBRARIES
      ${CMOCKA_LIBRARIES}
      ${CMOCKA_LIBRARY}
  )
endif (CMOCKA_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMocka DEFAULT_MSG CMOCKA_LIBRARIES CMOCKA_INCLUDE_DIR)

mark_as_advanced(CMOCKA_ROOT_DIR CMOCKA_INCLUDE_DIR CMOCKA_LIBRARIES)
