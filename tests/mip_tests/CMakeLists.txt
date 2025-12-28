add_executable(mipBackendTest
    mip_backend_test.cpp
)

target_include_directories(mipBackendTest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core/src
)

target_link_libraries(mipBackendTest
    routing_plugins
    routing
    gtest
    gtest_main
)

if(CPLEX_FOUND)
    target_link_libraries(mipBackendTest ${CPLEX_LIBRARIES})
endif()

if(HIGHS_FOUND)
    target_link_libraries(mipBackendTest ${HIGHS_LIBRARIES})
endif()

add_test(NAME mipBackendTest COMMAND mipBackendTest)
