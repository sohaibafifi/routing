set(GOOGLETEST_VERSION 1.10.0)
# Prefer in-tree Googletest headers to avoid mismatched system installs.
include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/googletest/googletest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/googletest/googletest
)
add_subdirectory(lib/googletest/googletest)
add_subdirectory(problem_tests)
add_subdirectory(composable_tests)
add_subdirectory(plugin_tests)
add_subdirectory(xcsp3)
add_subdirectory(cp_tests)
add_subdirectory(mip_tests)
