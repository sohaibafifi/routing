set(_GUROBI_HINTS "")
if(GUROBI_DIR)
    list(APPEND _GUROBI_HINTS "${GUROBI_DIR}")
endif()
if(GUROBI_HOME)
    list(APPEND _GUROBI_HINTS "${GUROBI_HOME}")
endif()
if(DEFINED ENV{GUROBI_HOME})
    list(APPEND _GUROBI_HINTS "$ENV{GUROBI_HOME}")
endif()
if(APPLE)
    file(GLOB _GUROBI_APPLE_ROOTS "/Library/gurobi*/macos_universal2")
    list(APPEND _GUROBI_HINTS ${_GUROBI_APPLE_ROOTS})
endif()
list(REMOVE_DUPLICATES _GUROBI_HINTS)

find_path(GUROBI_INCLUDE_DIRS
    NAMES gurobi_c++.h
    HINTS ${_GUROBI_HINTS}
    PATH_SUFFIXES include)

find_library(GUROBI_LIBRARY
    NAMES gurobi gurobi90 gurobi91 gurobi95 gurobi100 gurobi110 gurobi120 gurobi130 gurobi1301
    HINTS ${_GUROBI_HINTS}
    PATH_SUFFIXES lib)

if(MSVC)
    # determine Visual Studio year
    if(MSVC_TOOLSET_VERSION EQUAL 142)
        set(MSVC_YEAR "2019")
    elseif(MSVC_TOOLSET_VERSION EQUAL 141)
        set(MSVC_YEAR "2017")
    elseif(MSVC_TOOLSET_VERSION EQUAL 140)
        set(MSVC_YEAR "2015")
    endif()

    if(MT)
        set(M_FLAG "mt")
    else()
        set(M_FLAG "md")
    endif()

    find_library(GUROBI_CXX_LIBRARY
        NAMES gurobi_c++${M_FLAG}${MSVC_YEAR}
        HINTS ${_GUROBI_HINTS}
        PATH_SUFFIXES lib)
    find_library(GUROBI_CXX_DEBUG_LIBRARY
        NAMES gurobi_c++${M_FLAG}d${MSVC_YEAR}
        HINTS ${_GUROBI_HINTS}
        PATH_SUFFIXES lib)
else()
    find_library(GUROBI_CXX_LIBRARY
        NAMES gurobi_c++
        HINTS ${_GUROBI_HINTS}
        PATH_SUFFIXES lib)
    set(GUROBI_CXX_DEBUG_LIBRARY ${GUROBI_CXX_LIBRARY})
endif()

set(GUROBI_LIBRARIES ${GUROBI_CXX_LIBRARY} ${GUROBI_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GUROBI DEFAULT_MSG GUROBI_LIBRARY GUROBI_CXX_LIBRARY GUROBI_INCLUDE_DIRS)
