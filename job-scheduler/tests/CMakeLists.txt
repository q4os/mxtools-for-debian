find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(tst_crontime
    tst_crontime.cpp
    ${CMAKE_SOURCE_DIR}/src/CronTime.cpp
)
target_include_directories(tst_crontime PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_crontime PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_crontime COMMAND tst_crontime)

add_executable(tst_cronmodel
    tst_cronmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/CronModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Crontab.cpp
)
target_include_directories(tst_cronmodel PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_cronmodel PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_cronmodel COMMAND tst_cronmodel)

add_executable(tst_crontab
    tst_crontab.cpp
    ${CMAKE_SOURCE_DIR}/src/Crontab.cpp
)
target_include_directories(tst_crontab PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_crontab PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_crontab COMMAND tst_crontab)
