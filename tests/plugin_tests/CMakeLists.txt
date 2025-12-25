include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

add_executable(pluginRegistryTest registry_test.cpp)
target_include_directories(pluginRegistryTest PUBLIC ../../core/src/)
target_include_directories(pluginRegistryTest PUBLIC ../../)
target_link_libraries(pluginRegistryTest gtest gtest_main vrp)
add_test(pluginRegistryTest pluginRegistryTest)

add_executable(pluginAutoRegistrationTest auto_registration_test.cpp)
target_include_directories(pluginAutoRegistrationTest PUBLIC ../../core/src/)
target_include_directories(pluginAutoRegistrationTest PUBLIC ../../)
target_link_libraries(pluginAutoRegistrationTest gtest gtest_main routing_plugins)
add_test(pluginAutoRegistrationTest pluginAutoRegistrationTest)
