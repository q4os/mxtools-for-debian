# Tests CMakeLists.txt for mx-packageinstaller
cmake_minimum_required(VERSION 3.16)
project(mx-packageinstaller-tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Test module
find_package(Qt6 REQUIRED COMPONENTS Test Core Widgets)

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

# Test for PackageModel class
add_executable(test_packagemodel
    test_packagemodel.cpp
    ../src/models/packagemodel.cpp
    ../src/versionnumber.cpp
)

target_link_libraries(test_packagemodel
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_packagemodel PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_packagemodel PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_packagemodel PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME PackageModelTest COMMAND test_packagemodel)

# Test for FlatpakModel class
add_executable(test_flatpakmodel
    test_flatpakmodel.cpp
    ../src/models/flatpakmodel.cpp
)

target_link_libraries(test_flatpakmodel
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_flatpakmodel PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_flatpakmodel PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_flatpakmodel PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME FlatpakModelTest COMMAND test_flatpakmodel)

# Test for PackageFilterProxy class
add_executable(test_packagefilterproxy
    test_packagefilterproxy.cpp
    ../src/models/packagemodel.cpp
    ../src/models/packagefilterproxy.cpp
    ../src/versionnumber.cpp
)

target_link_libraries(test_packagefilterproxy
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_packagefilterproxy PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_packagefilterproxy PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_packagefilterproxy PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME PackageFilterProxyTest COMMAND test_packagefilterproxy)

# Test for FlatpakFilterProxy class
add_executable(test_flatpakfilterproxy
    test_flatpakfilterproxy.cpp
    ../src/models/flatpakmodel.cpp
    ../src/models/flatpakfilterproxy.cpp
)

target_link_libraries(test_flatpakfilterproxy
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_flatpakfilterproxy PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_flatpakfilterproxy PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_flatpakfilterproxy PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME FlatpakFilterProxyTest COMMAND test_flatpakfilterproxy)

# Test for PopularModel class
add_executable(test_popularmodel
    test_popularmodel.cpp
    ../src/models/popularmodel.cpp
)

target_link_libraries(test_popularmodel
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_popularmodel PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_popularmodel PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_popularmodel PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME PopularModelTest COMMAND test_popularmodel)

# Test for PopularFilterProxy class
add_executable(test_popularfilterproxy
    test_popularfilterproxy.cpp
    ../src/models/popularmodel.cpp
    ../src/models/popularfilterproxy.cpp
)

target_link_libraries(test_popularfilterproxy
    Qt6::Core
    Qt6::Test
    Qt6::Widgets
)

target_compile_options(test_popularfilterproxy PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(test_popularfilterproxy PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(test_popularfilterproxy PRIVATE -Werror=return-local-addr)
endif()

add_test(NAME PopularFilterProxyTest COMMAND test_popularfilterproxy)

# Add convenience target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_versionnumber test_aptcache test_packagemodel test_flatpakmodel
            test_packagefilterproxy test_flatpakfilterproxy test_popularmodel test_popularfilterproxy
    COMMENT "Running all tests"
)

# Add file watching target for continuous testing
add_custom_target(watch_tests
    COMMAND echo "Starting continuous testing - press Ctrl+C to stop"
    COMMAND bash -c "while inotifywait -e modify -r ${CMAKE_CURRENT_SOURCE_DIR}/.. --include='.*\\.(cpp|h)$$' 2>/dev/null; do echo ''; echo '=== Files changed, rebuilding and testing ==='; ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} && ${CMAKE_CTEST_COMMAND} --verbose; done"
    COMMENT "Watch for file changes and auto-run tests"
    USES_TERMINAL
)
