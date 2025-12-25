cmake_minimum_required(VERSION 3.12)
project(routing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set macOS architectures automatically when building on Apple platforms.
if(APPLE)
  if(NOT DEFINED CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL "")
    if(DEFINED CMAKE_HOST_SYSTEM_PROCESSOR)
      string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _proc)
      if(_proc MATCHES "^(aarch64|arm64|armv8)")
        set(_osx_arch "arm64")
      elseif(_proc MATCHES "^(x86_64|amd64)")
        set(_osx_arch "x86_64")
      else()
        set(_osx_arch "x86_64;arm64")
      endif()
    else()
      set(_osx_arch "x86_64;arm64")
    endif()
    set(CMAKE_OSX_ARCHITECTURES "${_osx_arch}" CACHE STRING "Auto-detected macOS architectures" FORCE)
  endif()
endif()

option(ROUTING_VERBOSE_MAKEFILE "Enable verbose build output" OFF)
if(ROUTING_VERBOSE_MAKEFILE)
  set(CMAKE_VERBOSE_MAKEFILE on)
endif()

option(ROUTING_BUILD_EXAMPLES "Build examples" ON)
option(ROUTING_BUILD_CPOPTIMIZER "Build the cpoptimizer CLI (requires CPLEX)" ON)
option(ROUTING_BUILD_GRB "Build the grb CLI (requires GUROBI)" ON)

include(CTest)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Helpers
function(routing_target_link_dl target_name)
  if(CMAKE_DL_LIBS)
    target_link_libraries(${target_name} PUBLIC ${CMAKE_DL_LIBS})
  endif()
endfunction()

find_package(Cplex REQUIRED)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})

find_package(GUROBI)
if(GUROBI_LIBRARY)
  message(STATUS "GUROBI_LIBRARY: ${GUROBI_LIBRARY}")
endif()

add_subdirectory(routing)
add_subdirectory(plugins)

if(ROUTING_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()