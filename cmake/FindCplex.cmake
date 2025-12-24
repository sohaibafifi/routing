# Source: https://github.com/martinsch/pgmlink

# This module finds cplex.
#
# User can give CPLEX_ROOT_DIR as a hint stored in the cmake cache.
#
# It sets the following variables:
#  CPLEX_FOUND              - Set to false, or undefined, if cplex isn't found.
#  CPLEX_INCLUDE_DIRS       - include directory
#  CPLEX_LIBRARIES          - library files

if(WIN32)
    execute_process(COMMAND cmd /C set CPLEX_STUDIO_DIR OUTPUT_VARIABLE CPLEX_STUDIO_DIR_VAR ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT CPLEX_STUDIO_DIR_VAR)
        MESSAGE(FATAL_ERROR "Unable to find CPLEX: environment variable CPLEX_STUDIO_DIR<VERSION> not set.")
    endif()

    STRING(REGEX REPLACE "^CPLEX_STUDIO_DIR" "" CPLEX_STUDIO_DIR_VAR ${CPLEX_STUDIO_DIR_VAR})
    STRING(REGEX MATCH "^[0-9]+" CPLEX_WIN_VERSION ${CPLEX_STUDIO_DIR_VAR})
    STRING(REGEX REPLACE "^[0-9]+=" "" CPLEX_STUDIO_DIR_VAR ${CPLEX_STUDIO_DIR_VAR})
    file(TO_CMAKE_PATH "${CPLEX_STUDIO_DIR_VAR}" CPLEX_ROOT_DIR_GUESS)

    set(CPLEX_WIN_VERSION ${CPLEX_WIN_VERSION} CACHE STRING "CPLEX version to be used.")
    set(CPLEX_ROOT_DIR "${CPLEX_ROOT_DIR_GUESS}" CACHE PATH "CPLEX root directory.")

    MESSAGE(STATUS "Found CLPEX version ${CPLEX_WIN_VERSION} at '${CPLEX_ROOT_DIR}'")

    STRING(REGEX REPLACE "/VC/bin/.*" "" VISUAL_STUDIO_PATH ${CMAKE_C_COMPILER})
    STRING(REGEX MATCH "Studio [0-9]+" CPLEX_WIN_VS_VERSION ${VISUAL_STUDIO_PATH})
    STRING(REGEX REPLACE "Studio " "" CPLEX_WIN_VS_VERSION ${CPLEX_WIN_VS_VERSION})

    if(${CPLEX_WIN_VS_VERSION} STREQUAL "9")
        set(CPLEX_WIN_VS_VERSION 2008)
    elseif(${CPLEX_WIN_VS_VERSION} STREQUAL "10")
        set(CPLEX_WIN_VS_VERSION 2010)
    elseif(${CPLEX_WIN_VS_VERSION} STREQUAL "11")
        set(CPLEX_WIN_VS_VERSION 2012)
    else()
        MESSAGE(FATAL_ERROR "CPLEX: unknown Visual Studio version at '${VISUAL_STUDIO_PATH}'.")
    endif()

    set(CPLEX_WIN_VS_VERSION ${CPLEX_WIN_VS_VERSION} CACHE STRING "Visual Studio Version")

    if("${CMAKE_C_COMPILER}" MATCHES "amd64")
        set(CPLEX_WIN_BITNESS x64)
    else()
        set(CPLEX_WIN_BITNESS x86)
    endif()

    set(CPLEX_WIN_BITNESS ${CPLEX_WIN_BITNESS} CACHE STRING "On Windows: x86 or x64 (32bit resp. 64bit)")

    MESSAGE(STATUS "CPLEX: using Visual Studio ${CPLEX_WIN_VS_VERSION} ${CPLEX_WIN_BITNESS} at '${VISUAL_STUDIO_PATH}'")

    if(NOT CPLEX_WIN_LINKAGE)
        set(CPLEX_WIN_LINKAGE mda CACHE STRING "CPLEX linkage variant on Windows. One of these: mda (dll, release), mdd (dll, debug), mta (static, release), mtd (static, debug)")
    endif(NOT CPLEX_WIN_LINKAGE)

    # now, generate platform string
    set(CPLEX_WIN_PLATFORM "${CPLEX_WIN_BITNESS}_windows_vs${CPLEX_WIN_VS_VERSION}/stat_${CPLEX_WIN_LINKAGE}")

else()

    set(CPLEX_ROOT_DIR "" CACHE PATH "CPLEX root directory.")
    set(CPLEX_WIN_PLATFORM "")

endif()

# If user didn't set CPLEX_ROOT_DIR earlier, default hints were emitted above.
# Detect host/target architecture to add arm64/aarch64 search paths for CPLEX (Apple Silicon, aarch64 Linux).
if(NOT DEFINED CPLEX_ARCH)
    set(_CPLEX__PROC "")
    if(APPLE AND DEFINED CMAKE_OSX_ARCHITECTURES AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
        set(_CPLEX__ARCH_LIST ${CMAKE_OSX_ARCHITECTURES})
        list(GET _CPLEX__ARCH_LIST 0 _CPLEX__PROC)
    elseif(DEFINED CMAKE_SYSTEM_PROCESSOR)
        set(_CPLEX__PROC ${CMAKE_SYSTEM_PROCESSOR})
    elseif(DEFINED CMAKE_HOST_SYSTEM_PROCESSOR)
        set(_CPLEX__PROC ${CMAKE_HOST_SYSTEM_PROCESSOR})
    endif()
    string(TOLOWER "${_CPLEX__PROC}" _CPLEX__PROC_LOWER)

    if(_CPLEX__PROC_LOWER MATCHES "aarch64|arm64|armv8")
        set(CPLEX_ARCH "arm64")
    elseif(_CPLEX__PROC_LOWER MATCHES "x86_64|amd64|i686")
        set(CPLEX_ARCH "x86-64")
    else()
        set(CPLEX_ARCH "")
    endif()
endif()

if(NOT  CPLEX_ROOT_DIR)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(_CPLEX_DEFAULT_ROOT_DIR "/Applications/CPLEX_Studio2211/")
    else ()
        set(_CPLEX_DEFAULT_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio2211")
    endif ()
    set(CPLEX_ROOT_DIR "${_CPLEX_DEFAULT_ROOT_DIR}" CACHE PATH "CPLEX root directory." FORCE)
    if(EXISTS "${CPLEX_ROOT_DIR}")
        message(STATUS "CPLEX_ROOT_DIR not set; using default: ${CPLEX_ROOT_DIR}")
    else()
        message(WARNING "CPLEX_ROOT_DIR not set. Please set it to the root directory of your CPLEX installation.")
        message(STATUS "Tried default CPLEX_ROOT_DIR : ${CPLEX_ROOT_DIR}")
    endif()

else()
    message(STATUS "CPLEX_ROOT_DIR : ${CPLEX_ROOT_DIR}")
endif()

FIND_PATH(CPLEX_INCLUDE_DIR
        ilcplex/cplex.h
        HINTS ${CPLEX_ROOT_DIR}/cplex/include
        ${CPLEX_ROOT_DIR}/include
        PATHS ENV C_INCLUDE_PATH
        ENV C_PLUS_INCLUDE_PATH
        ENV INCLUDE_PATH
        )

FIND_PATH(CPOPTIMIZER_INCLUDE_DIR
        ilcp/cp.h
        HINTS ${CPLEX_ROOT_DIR}/cpoptimizer/include
        ${CPLEX_ROOT_DIR}/include
        PATHS ENV C_INCLUDE_PATH
        ENV C_PLUS_INCLUDE_PATH
        ENV INCLUDE_PATH
        )

FIND_PATH(CPLEX_CONCERT_INCLUDE_DIR
        ilconcert/iloenv.h
        HINTS ${CPLEX_ROOT_DIR}/concert/include
        ${CPLEX_ROOT_DIR}/include
        PATHS ENV C_INCLUDE_PATH
        ENV C_PLUS_INCLUDE_PATH
        ENV INCLUDE_PATH
        )

message(STATUS "CPLEX Include Dir: ${CPLEX_INCLUDE_DIR}")
message(STATUS "CPLEX CONCERT Include Dir: ${CPLEX_CONCERT_INCLUDE_DIR}")
message(STATUS "CPLEX CP Optimizer Include Dir: ${CPOPTIMIZER_INCLUDE_DIR}")
FIND_LIBRARY(CPLEX_LIBRARY
        NAMES cplex${CPLEX_WIN_VERSION} cplex
        HINTS
        # Preferred: match CPLEX lookup to the configured target architecture.
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_linux
        # ARM64 / Apple Silicon / aarch64 Linux variants
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}/static_pic # generic arch dir if present
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_linux
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_linux
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH} # generic arch dir if present
        # windows and x86-64 unix/osx variants (existing)
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_WIN_PLATFORM} #windows
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_debian4.0_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_sles10_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_osx/static_pic #osx
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_darwin/static_pic #osx
        PATHS ENV LIBRARY_PATH #unix
        ENV LD_LIBRARY_PATH #unix
        )
message(STATUS "CPLEX Library: ${CPLEX_LIBRARY}")

FIND_LIBRARY(CPLEX_ILOCPLEX_LIBRARY
        ilocplex
        HINTS
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}_linux
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}/static_pic
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_osx
        ${CPLEX_ROOT_DIR}/cplex/lib/aarch64_linux
        ${CPLEX_ROOT_DIR}/cplex/lib/arm64_linux
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_ARCH}
        ${CPLEX_ROOT_DIR}/cplex/lib/${CPLEX_WIN_PLATFORM} #windows
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_debian4.0_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_sles10_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic #unix
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_osx/static_pic #osx
        ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_darwin/static_pic #osx
        PATHS ENV LIBRARY_PATH
        ENV LD_LIBRARY_PATH
        )

message(STATUS "ILOCPLEX Library: ${CPLEX_ILOCPLEX_LIBRARY}")

FIND_LIBRARY(CPLEX_CONCERT_LIBRARY
        concert
        HINTS
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}_osx/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}_osx
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}_linux/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}_linux
        ${CPLEX_ROOT_DIR}/concert/lib/arm64_osx/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/aarch64_osx/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/aarch64_linux/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/arm64_linux/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}/static_pic
        ${CPLEX_ROOT_DIR}/concert/lib/arm64_osx
        ${CPLEX_ROOT_DIR}/concert/lib/aarch64_osx
        ${CPLEX_ROOT_DIR}/concert/lib/aarch64_linux
        ${CPLEX_ROOT_DIR}/concert/lib/arm64_linux
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_ARCH}
        ${CPLEX_ROOT_DIR}/concert/lib/${CPLEX_WIN_PLATFORM} #windows
        ${CPLEX_ROOT_DIR}/concert/lib/x86-64_debian4.0_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/concert/lib/x86-64_sles10_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/concert/lib/x86-64_linux/static_pic #unix
        ${CPLEX_ROOT_DIR}/concert/lib/x86-64_osx/static_pic #osx
        ${CPLEX_ROOT_DIR}/concert/lib/x86-64_darwin/static_pic #osx
        PATHS ENV LIBRARY_PATH
        ENV LD_LIBRARY_PATH
        )
message(STATUS "CONCERT Library: ${CPLEX_CONCERT_LIBRARY}")

FIND_LIBRARY(CPLEX_CPOPTIMIZER_LIBRARY
        cp
        HINTS
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}_osx/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}_osx
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}_linux/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}_linux
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/arm64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/aarch64_osx/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/aarch64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/arm64_linux/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}/static_pic
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/arm64_osx
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/aarch64_osx
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/aarch64_linux
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/arm64_linux
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_ARCH}
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/${CPLEX_WIN_PLATFORM} #windows
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/x86-64_debian4.0_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/x86-64_sles10_4.1/static_pic #unix
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/x86-64_linux/static_pic #unix
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/x86-64_osx/static_pic #osx
        ${CPLEX_ROOT_DIR}/cpoptimizer/lib/x86-64_darwin/static_pic #osx
        PATHS ENV LIBRARY_PATH
        ENV LD_LIBRARY_PATH
        )
message(STATUS "CP Library: ${CPLEX_CPOPTIMIZER_LIBRARY}")
if(WIN32)
    FIND_PATH(CPLEX_BIN_DIR
            cplex${CPLEX_WIN_VERSION}.dll
            HINTS ${CPLEX_ROOT_DIR}/cplex/bin/${CPLEX_WIN_PLATFORM} #windows
            )
else()
    FIND_PATH(CPLEX_BIN_DIR
            cplex
            HINTS
            ${CPLEX_ROOT_DIR}/cplex/bin/${CPLEX_ARCH}_osx
            ${CPLEX_ROOT_DIR}/cplex/bin/${CPLEX_ARCH}_linux
            ${CPLEX_ROOT_DIR}/cplex/bin/arm64_osx
            ${CPLEX_ROOT_DIR}/cplex/bin/aarch64_osx
            ${CPLEX_ROOT_DIR}/cplex/bin/aarch64_linux
            ${CPLEX_ROOT_DIR}/cplex/bin/arm64_linux
            ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_sles10_4.1 #unix
            ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_debian4.1
            ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux #unix
            ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_osx #osx
            ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_darwin #osx
            ENV LIBRARY_PATH
            ENV LD_LIBRARY_PATH
            )
endif()
message(STATUS "CPLEX Bin Dir: ${CPLEX_BIN_DIR}")

# Finalize found status and variables
set(CPLEX_FOUND FALSE)
if(CPLEX_INCLUDE_DIR AND CPLEX_LIBRARY AND CPLEX_ILOCPLEX_LIBRARY AND CPLEX_CONCERT_LIBRARY)
    set(CPLEX_FOUND TRUE)
endif()

if(CPLEX_FOUND)
    message(STATUS "CPLEX: found (arch hint: ${CPLEX_ARCH})")
    # Expose standardized variables expected by projects
    set(CPLEX_INCLUDE_DIRS ${CPLEX_INCLUDE_DIR} ${CPLEX_CONCERT_INCLUDE_DIR} ${CPOPTIMIZER_INCLUDE_DIR} CACHE PATH "CPLEX include directories" FORCE)
    set(CPLEX_LIBRARIES ${CPLEX_LIBRARY} ${CPLEX_ILOCPLEX_LIBRARY} ${CPLEX_CONCERT_LIBRARY} ${CPLEX_CPOPTIMIZER_LIBRARY} CACHE PATH "CPLEX libraries" FORCE)
    set(CPLEX_BIN_DIR ${CPLEX_BIN_DIR} CACHE PATH "CPLEX bin directory" FORCE)
    add_definitions(-DCPLEX_FOUND)
else()
    message(WARNING "CPLEX not found. Make sure CPLEX_ROOT_DIR is set and points to a valid CPLEX installation (for arm64 look under cplex/lib/arm64* or aarch64*).")
    set(CPLEX_FOUND FALSE CACHE BOOL "CPLEX found")
endif()
