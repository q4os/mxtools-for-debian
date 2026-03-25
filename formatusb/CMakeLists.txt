cmake_minimum_required(VERSION 3.16)

project(formatusb VERSION 25.06.01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 first, fallback to Qt5
find_package(Qt6 COMPONENTS Core Widgets LinguistTools)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Widgets LinguistTools REQUIRED)
endif()

# Enable Qt automoc, autorcc, autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define the executable
add_executable(formatusb
    main.cpp
    mainwindow.cpp
    about.cpp
    cmd.cpp
    mainwindow.ui
    images.qrc
)

# Add headers (for IDE support)
target_sources(formatusb PRIVATE
    mainwindow.h
    version.h
    about.h
    cmd.h
)

# Link Qt libraries
target_link_libraries(formatusb
    Qt::Core
    Qt::Widgets
)

# Include directories
target_include_directories(formatusb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install target
install(TARGETS formatusb
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Handle translations
file(GLOB TS_FILES translations/*.ts)
if(Qt6_FOUND)
    qt_add_translations(formatusb TS_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    target_sources(formatusb PRIVATE ${QM_FILES})
endif()