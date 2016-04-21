INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_PROTOTYPE prototype)

FIND_PATH(
    PROTOTYPE_INCLUDE_DIRS
    NAMES prototype/api.h
    HINTS $ENV{PROTOTYPE_DIR}/include
        ${PC_PROTOTYPE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    PROTOTYPE_LIBRARIES
    NAMES gnuradio-prototype
    HINTS $ENV{PROTOTYPE_DIR}/lib
        ${PC_PROTOTYPE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROTOTYPE DEFAULT_MSG PROTOTYPE_LIBRARIES PROTOTYPE_INCLUDE_DIRS)
MARK_AS_ADVANCED(PROTOTYPE_LIBRARIES PROTOTYPE_INCLUDE_DIRS)

