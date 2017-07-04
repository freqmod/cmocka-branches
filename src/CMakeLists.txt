project(cmocka-branches-library C)

set(CMOCKA_BRANCHES_PLATFORM_INCLUDE CACHE PATH "Path to include directory for cmocka_branches_platform.h")

set(CMOCKA_BRANCHES_PUBLIC_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMOCKA_BRANCHES_PLATFORM_INCLUDE}
    CACHE INTERNAL "cmocka public include directories"
)

set(CMOCKA_BRANCHES_PRIVATE_INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}
)

set(CMOCKA_BRANCHES_SHARED_LIBRARY
    cmocka_branches_shared
    CACHE INTERNAL "cmocka shared library"
)

if (WITH_STATIC_LIB)
    set(CMOCKA_BRANCHES_STATIC_LIBRARY
        cmocka_branches_static
        CACHE INTERNAL "cmocka static library"
    )
endif (WITH_STATIC_LIB)

set(CMOCKA_BRANCHES_LINK_LIBRARIES
    ${CMOCKA_BRANCHES_REQUIRED_LIBRARIES}
    CACHE INTERNAL "cmocka link libraries"
)

set(CMOCKA_BRANCHES_SRCS
    cmocka_branches.c
)

if (WIN32)
    set(CMOCKA_BRANCHES_SRCS
        ${CMOCKA_BRANCHES_SRCS}
        cmocka_branches.def
    )
endif (WIN32)

include_directories(
    ${CMOCKA_BRANCHES_PUBLIC_INCLUDE_DIRS}
    ${CMOCKA_BRANCHES_PRIVATE_INCLUDE_DIRS}
    ${CMOCKA_INCLUDE_DIR}
)

add_definitions(-DHAVE_CONFIG_H=1)
if (CMOCKA_BRANCHES_PLATFORM_INCLUDE)
    add_definitions(-DCMOCKA_BRANCHES_PLATFORM_INCLUDE=1)
endif()

add_library(${CMOCKA_BRANCHES_SHARED_LIBRARY} SHARED ${CMOCKA_BRANCHES_SRCS})

target_link_libraries(${CMOCKA_BRANCHES_SHARED_LIBRARY} ${CMOCKA_BRANCHES_LINK_LIBRARIES} ${CMOCKA_LIBRARIES})
set_target_properties(
    ${CMOCKA_BRANCHES_SHARED_LIBRARY}
        PROPERTIES
            OUTPUT_NAME
                cmocka_branches
            DEFINE_SYMBOL
                CMOCKA_BRANCHES_EXPORTS
)

if (NOT WIN32)
    set_target_properties(
        ${CMOCKA_BRANCHES_SHARED_LIBRARY}
            PROPERTIES
                VERSION
                    ${LIBRARY_VERSION}
                SOVERSION
                    ${LIBRARY_SOVERSION}
    )
endif (NOT WIN32)

install(
    TARGETS ${CMOCKA_BRANCHES_SHARED_LIBRARY}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT libraries
)

if (WITH_STATIC_LIB)
    add_library(${CMOCKA_BRANCHES_STATIC_LIBRARY} STATIC ${CMOCKA_BRANCHES_SRCS} ${CMOCKA_LIBRARIES})

    set_target_properties(
        ${CMOCKA_BRANCHES_STATIC_LIBRARY}
            PROPERTIES
                VERSION
                    ${LIBRARY_VERSION}
                SOVERSION
                    ${LIBRARY_SOVERSION}
                OUTPUT_NAME
                    cmocka_branches
    )

    install(
        TARGETS ${CMOCKA_BRANCHES_STATIC_LIBRARY}
        DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT libraries
    )
endif (WITH_STATIC_LIB)

if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()

#
# In order to run tests we will need to set the approriate environment
# variable so that the test program can locate its dependent DLL's. First
# we want to know what directory our dependent DLL was installed into:
#
get_target_property(_cmocka_branches_dir cmocka_branches_shared LOCATION_${CMOCKA_BRANCHES_BUILD_TYPE})
get_filename_component(_cmocka_branches_path "${_cmocka_branches_dir}" PATH)
file(TO_NATIVE_PATH "${_cmocka_branches_path}" _cmocka_branches_dir_path_native)
file(TO_NATIVE_PATH "${_cmocka_branches_dir}"  _cmocka_branches_path_native)

set(CMOCKA_BRANCHES_DLL_DIR_PATH "${_cmocka_branches_dir_path_native}" PARENT_SCOPE)
set(CMOCKA_BRANCHES_DLL_PATH "${_cmocka_branches_path_native}" PARENT_SCOPE)
