# FindORTools.cmake
# Find the OR-Tools optimization library (Google)
#
# This module defines:
#   ORTOOLS_FOUND        - True if OR-Tools was found
#   ORTOOLS_INCLUDE_DIRS - OR-Tools include directories
#   ORTOOLS_LIBRARIES    - OR-Tools libraries to link
#   ORTOOLS_VERSION      - OR-Tools version string
#
# The following environment variables are checked:
#   ORTOOLS_HOME, ORTOOLS_DIR, ORTOOLS_ROOT
#
# The following CMake variables can be set:
#   ORTOOLS_ROOT - Root directory of OR-Tools installation

# Try to find OR-Tools using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_ORTOOLS QUIET ortools)
endif()

# Find include directory
find_path(ORTOOLS_INCLUDE_DIR
    NAMES ortools/sat/cp_model.h
    HINTS
        ${PC_ORTOOLS_INCLUDEDIR}
        ${PC_ORTOOLS_INCLUDE_DIRS}
        ${ORTOOLS_ROOT}
        ${ORTOOLS_HOME}
        ${ORTOOLS_DIR}
        ENV ORTOOLS_ROOT
        ENV ORTOOLS_HOME
        ENV ORTOOLS_DIR
    PATH_SUFFIXES
        include
)

# Find library
find_library(ORTOOLS_LIBRARY
    NAMES ortools
    HINTS
        ${PC_ORTOOLS_LIBDIR}
        ${PC_ORTOOLS_LIBRARY_DIRS}
        ${ORTOOLS_ROOT}
        ${ORTOOLS_HOME}
        ${ORTOOLS_DIR}
        ENV ORTOOLS_ROOT
        ENV ORTOOLS_HOME
        ENV ORTOOLS_DIR
    PATH_SUFFIXES
        lib
        lib64
        lib/x86_64-linux-gnu
)

# Try to get version
if(ORTOOLS_INCLUDE_DIR)
    file(STRINGS "${ORTOOLS_INCLUDE_DIR}/ortools/base/version.h" ORTOOLS_VERSION_LINE
         REGEX "#define OR_TOOLS_.*_VERSION ")
    if(ORTOOLS_VERSION_LINE)
        string(REGEX MATCH "OR_TOOLS_MAJOR_VERSION ([0-9]+)" _ "${ORTOOLS_VERSION_LINE}")
        set(ORTOOLS_VERSION_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "OR_TOOLS_MINOR_VERSION ([0-9]+)" _ "${ORTOOLS_VERSION_LINE}")
        set(ORTOOLS_VERSION_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "OR_TOOLS_PATCH_VERSION ([0-9]+)" _ "${ORTOOLS_VERSION_LINE}")
        set(ORTOOLS_VERSION_PATCH ${CMAKE_MATCH_1})
        if(ORTOOLS_VERSION_MAJOR AND ORTOOLS_VERSION_MINOR)
            set(ORTOOLS_VERSION "${ORTOOLS_VERSION_MAJOR}.${ORTOOLS_VERSION_MINOR}")
            if(ORTOOLS_VERSION_PATCH)
                set(ORTOOLS_VERSION "${ORTOOLS_VERSION}.${ORTOOLS_VERSION_PATCH}")
            endif()
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ORTools
    REQUIRED_VARS ORTOOLS_LIBRARY ORTOOLS_INCLUDE_DIR
    VERSION_VAR ORTOOLS_VERSION
)

if(ORTOOLS_FOUND)
    set(ORTOOLS_LIBRARIES ${ORTOOLS_LIBRARY})
    set(ORTOOLS_INCLUDE_DIRS ${ORTOOLS_INCLUDE_DIR})

    # OR-Tools headers pull in Abseil/Protobuf symbols that may need explicit linking.
    set(_ortools_extra_libs "")
    find_package(absl CONFIG QUIET)
    if(TARGET absl::flat_hash_map)
        list(APPEND _ortools_extra_libs absl::flat_hash_map)
    elseif(TARGET absl::raw_hash_set)
        list(APPEND _ortools_extra_libs absl::raw_hash_set)
    elseif(TARGET absl::raw_logging_internal)
        list(APPEND _ortools_extra_libs absl::raw_logging_internal)
    endif()

    find_package(Protobuf QUIET CONFIG)
    if(TARGET protobuf::libprotobuf)
        list(APPEND _ortools_extra_libs protobuf::libprotobuf)
    else()
        find_package(Protobuf QUIET)
        if(Protobuf_FOUND)
            list(APPEND _ortools_extra_libs ${Protobuf_LIBRARIES})
        endif()
    endif()

    if(_ortools_extra_libs)
        list(APPEND ORTOOLS_LIBRARIES ${_ortools_extra_libs})
    endif()

    if(NOT TARGET ORTools::ORTools)
        add_library(ORTools::ORTools UNKNOWN IMPORTED)
        set_target_properties(ORTools::ORTools PROPERTIES
            IMPORTED_LOCATION "${ORTOOLS_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ORTOOLS_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${_ortools_extra_libs}"
        )
    endif()

    message(STATUS "Found OR-Tools: ${ORTOOLS_LIBRARY} (version ${ORTOOLS_VERSION})")
endif()

mark_as_advanced(ORTOOLS_INCLUDE_DIR ORTOOLS_LIBRARY)
