# Incremental Evaluation Tests

add_executable(incrementalEvaluatorTest
    incremental_evaluator_test.cpp
)

target_include_directories(incrementalEvaluatorTest PRIVATE
    ${PROJECT_SOURCE_DIR}/core/src
    ${PROJECT_SOURCE_DIR}/plugins
)

target_link_libraries(incrementalEvaluatorTest
    routing
    routing_plugins
    gtest
    gtest_main
)

add_test(NAME incrementalEvaluatorTest COMMAND incrementalEvaluatorTest)
