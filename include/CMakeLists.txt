project(cmocka-branches-headers C)

set(CMOCKA_BRANCHES_HDRS
  cmocka_branches.h
)

install(
  FILES
    ${CMOCKA_BRANCHES_HDRS}
  DESTINATION
    ${INCLUDE_INSTALL_DIR}
  COMPONENT
    headers
)
