# Benchmark CLI executable
# Usage: ./benchmark --instance R101.txt --solver ga --timeout 60

add_executable(benchmark main.cpp)

target_link_libraries(benchmark PRIVATE
    routing_core
    routing_plugins
)

target_include_directories(benchmark PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/argparse/include
    ${PROJECT_SOURCE_DIR}/core/src
    ${PROJECT_SOURCE_DIR}/plugins
)

# Install
install(TARGETS benchmark RUNTIME DESTINATION bin)
