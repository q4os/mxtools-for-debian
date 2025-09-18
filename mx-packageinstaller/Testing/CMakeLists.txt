# Tests CMakeLists.txt for mx-packageinstaller
cmake_minimum_required(VERSION 3.16)
project(mx-packageinstaller-tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Test module
find_package(Qt6 REQUIRED COMPONENTS Test Core)

# Enable testing
enable_testing()

# Set up test include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
set(CMAKE_AUTOMOC ON)

# Test for VersionNumber class
add_executable(test_versionnumber
    test_versionnumber.cpp
    ../src/versionnumber.cpp
)

target_link_libraries(test_versionnumber
    Qt6::Core
    Qt6::Test
)

target_compile_options(test_versionnumber PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

# Add compiler-specific flags for tests
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_versionnumber PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_versionnumber PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME VersionNumberTest COMMAND test_versionnumber)

# Test for AptCache class
add_executable(test_aptcache
    test_aptcache.cpp
    ../src/aptcache.cpp
    ../src/versionnumber.cpp
)

target_link_libraries(test_aptcache
    Qt6::Core
    Qt6::Test
)

target_compile_options(test_aptcache PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

# Add compiler-specific flags for tests
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_aptcache PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_aptcache PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME AptCacheTest COMMAND test_aptcache)

# Add convenience target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_versionnumber test_aptcache
    COMMENT "Running all tests"
)

# Add file watching target for continuous testing
add_custom_target(watch_tests
    COMMAND echo "Starting continuous testing - press Ctrl+C to stop"
    COMMAND bash -c "while inotifywait -e modify -r ${CMAKE_CURRENT_SOURCE_DIR}/.. --include='.*\\.(cpp|h)$$' 2>/dev/null; do echo ''; echo '=== Files changed, rebuilding and testing ==='; ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} && ${CMAKE_CTEST_COMMAND} --verbose; done"
    COMMENT "Watch for file changes and auto-run tests"
    USES_TERMINAL
)