find_package(Qt6 REQUIRED COMPONENTS Core Test)

add_executable(test_parsing
    test_parsing.cpp
    ${CMAKE_SOURCE_DIR}/src/cardutils.cpp
)

target_include_directories(test_parsing PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(test_parsing
    Qt6::Core
    Qt6::Test
)

target_compile_definitions(test_parsing PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

add_test(NAME test_parsing COMMAND test_parsing)
