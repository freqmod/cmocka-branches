# - Try to find CMocka
# Once done this will define
#
#  CMOCKA_BRANCHES_ROOT_DIR - Set this variable to the root installation of CMocka
#
# Read-Only variables:
#  CMOCKA_BRANCHES_FOUND - system has CMocka
#  CMOCKA_BRANCHES_INCLUDE_DIR - the CMocka include directory
#  CMOCKA_BRANCHES_LIBRARIES - Link these to use CMocka
#  CMOCKA_BRANCHES_DEFINITIONS - Compiler switches required for using CMocka

set(_CMOCKA_BRANCHES_ROOT_HINTS
)

set(_CMOCKA_BRANCHES_ROOT_PATHS
    "$ENV{PROGRAMFILES}/cmocka_branches"
    "C:/dev/cmbranches"
)

find_path(CMOCKA_BRANCHES_ROOT_DIR
    NAMES
        include/cmocka_branches.h
    HINTS
        ${_CMOCKA_BRANCHES_ROOT_HINTS}
    PATHS
        ${_CMOCKA_BRANCHES_ROOT_PATHS}
)

find_path(CMOCKA_BRANCHES_INCLUDE_DIR
    NAMES
        cmocka_branches.h
    PATHS
        ${CMOCKA_BRANCHES_ROOT_DIR}/include
)

find_library(CMOCKA_BRANCHES_LIBRARY
    NAMES
        cmocka_branches
    PATHS
        ${CMOCKA_BRANCHES_ROOT_DIR}/lib
         /usr/lib/i386-linux-gnu
)

if (CMOCKA_BRANCHES_LIBRARY)
  set(CMOCKA_BRANCHES_LIBRARIES
      ${CMOCKA_BRANCHES_LIBRARIES}
      ${CMOCKA_BRANCHES_LIBRARY}
  )
endif (CMOCKA_BRANCHES_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMockaBranches DEFAULT_MSG CMOCKA_BRANCHES_LIBRARIES CMOCKA_BRANCHES_INCLUDE_DIR)

mark_as_advanced(CMOCKA_BRANCHES_ROOT_DIR CMOCKA_BRANCHES_INCLUDE_DIR CMOCKA_BRANCHES_LIBRARIES)
