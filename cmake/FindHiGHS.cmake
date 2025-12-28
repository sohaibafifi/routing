# FindHiGHS.cmake
# Find the HiGHS linear and mixed-integer programming solver
#
# This module defines:
#   HIGHS_FOUND        - True if HiGHS was found
#   HIGHS_INCLUDE_DIRS - HiGHS include directories
#   HIGHS_LIBRARIES    - HiGHS libraries to link
#   HIGHS_VERSION      - HiGHS version string
#
# The following environment variables are checked:
#   HIGHS_HOME, HIGHS_DIR, HIGHS_ROOT
#
# The following CMake variables can be set:
#   HIGHS_ROOT - Root directory of HiGHS installation

# Try to find HiGHS using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_HIGHS QUIET highs)
endif()

# Find include directory
find_path(HIGHS_INCLUDE_DIR
    NAMES Highs.h
    HINTS
        ${PC_HIGHS_INCLUDEDIR}
        ${PC_HIGHS_INCLUDE_DIRS}
        ${HIGHS_ROOT}
        ${HIGHS_HOME}
        ${HIGHS_DIR}
        ENV HIGHS_ROOT
        ENV HIGHS_HOME
        ENV HIGHS_DIR
    PATH_SUFFIXES
        include
        include/highs
        highs
)

# Find library
find_library(HIGHS_LIBRARY
    NAMES highs
    HINTS
        ${PC_HIGHS_LIBDIR}
        ${PC_HIGHS_LIBRARY_DIRS}
        ${HIGHS_ROOT}
        ${HIGHS_HOME}
        ${HIGHS_DIR}
        ENV HIGHS_ROOT
        ENV HIGHS_HOME
        ENV HIGHS_DIR
    PATH_SUFFIXES
        lib
        lib64
        lib/x86_64-linux-gnu
)

# Try to get version
if(HIGHS_INCLUDE_DIR)
    file(STRINGS "${HIGHS_INCLUDE_DIR}/HConfig.h" HIGHS_VERSION_LINE
         REGEX "#define HIGHS_VERSION_.*")
    if(HIGHS_VERSION_LINE)
        string(REGEX MATCH "HIGHS_VERSION_MAJOR ([0-9]+)" _ ${HIGHS_VERSION_LINE})
        set(HIGHS_VERSION_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "HIGHS_VERSION_MINOR ([0-9]+)" _ ${HIGHS_VERSION_LINE})
        set(HIGHS_VERSION_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "HIGHS_VERSION_PATCH ([0-9]+)" _ ${HIGHS_VERSION_LINE})
        set(HIGHS_VERSION_PATCH ${CMAKE_MATCH_1})
        set(HIGHS_VERSION "${HIGHS_VERSION_MAJOR}.${HIGHS_VERSION_MINOR}.${HIGHS_VERSION_PATCH}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HiGHS
    REQUIRED_VARS HIGHS_LIBRARY HIGHS_INCLUDE_DIR
    VERSION_VAR HIGHS_VERSION
)

if(HIGHS_FOUND)
    set(HIGHS_LIBRARIES ${HIGHS_LIBRARY})
    set(HIGHS_INCLUDE_DIRS ${HIGHS_INCLUDE_DIR})

    if(NOT TARGET HiGHS::HiGHS)
        add_library(HiGHS::HiGHS UNKNOWN IMPORTED)
        set_target_properties(HiGHS::HiGHS PROPERTIES
            IMPORTED_LOCATION "${HIGHS_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${HIGHS_INCLUDE_DIR}"
        )
    endif()

    message(STATUS "Found HiGHS: ${HIGHS_LIBRARY} (version ${HIGHS_VERSION})")
endif()

mark_as_advanced(HIGHS_INCLUDE_DIR HIGHS_LIBRARY)
