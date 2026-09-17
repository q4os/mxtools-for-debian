# **********************************************************************
# * Copyright (C) 2017-2026 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-welcome.
# *
# * mx-welcome is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-welcome is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-welcome.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

cmake_minimum_required(VERSION 3.16)

# Get version from debian/changelog
execute_process(
    COMMAND dpkg-parsechangelog -SVersion
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION_FROM_CHANGELOG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE DPKG_RESULT
)

if(NOT DPKG_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to get version from debian/changelog using dpkg-parsechangelog")
endif()

# Convert Debian package version into a valid CMake project version.
# Accepts both YY.MMmxNN and YY.MM.PPmxNN and normalizes to YY.MM.PP.
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" PROJECT_VERSION_MATCH "${PROJECT_VERSION_FROM_CHANGELOG}")

if(NOT PROJECT_VERSION_MATCH)
    message(FATAL_ERROR "Unsupported version format from debian/changelog: ${PROJECT_VERSION_FROM_CHANGELOG}")
endif()

set(PROJECT_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(PROJECT_VERSION_MINOR "${CMAKE_MATCH_2}")

if(DEFINED CMAKE_MATCH_4 AND NOT CMAKE_MATCH_4 STREQUAL "")
    set(PROJECT_VERSION_PATCH "${CMAKE_MATCH_4}")
else()
    set(PROJECT_VERSION_PATCH "0")
endif()

set(PROJECT_VERSION_CMAKE "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

project(mx-welcome
    VERSION ${PROJECT_VERSION_CMAKE}
    DESCRIPTION "MX Welcome - Welcome application for MX Linux"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile commands export for IDEs and tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimize for Ninja builds
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # Enable colored output for Ninja
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    LinguistTools
)

# These policies were introduced after the minimum Qt version available on
# some supported distributions. Only set them when the installed Qt knows
# about them.
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.5)
    qt6_policy(SET QTP0001 NEW)
endif()
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.8)
    qt6_policy(SET QTP0004 NEW)
endif()

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

# Define source files
set(SOURCES
    src/main.cpp
    src/actionmodel.cpp
    src/backend.cpp
)

set(HEADERS
    src/actionmodel.h
    src/backend.h
)

# No resource files for mx-welcome currently
# set(RESOURCE_FILES
#     images.qrc
# )

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")
list(FILTER TRANSLATION_FILES EXCLUDE REGEX "/mx-welcome_en_US\\.ts$")

# Create the executable
add_executable(mx-welcome
    ${SOURCES}
    ${HEADERS}
)

qt6_add_qml_module(mx-welcome
    URI MxWelcome
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/SecondaryButton.qml
        qml/components/ToolCard.qml
    RESOURCES
        icons/mx-welcome.svg
)

# Link Qt6 libraries
target_link_libraries(mx-welcome
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# Set compiler flags
target_compile_options(mx-welcome PRIVATE
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
    -Werror
)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(mx-welcome PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(mx-welcome PRIVATE -Werror=return-local-addr)
endif()

# Set compile definitions
target_compile_definitions(mx-welcome PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(mx-welcome PRIVATE NDEBUG)
    target_compile_options(mx-welcome PRIVATE -Os)

    # Add LTO - different flags for different compilers
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(mx-welcome PRIVATE -flto=thin)
        target_link_options(mx-welcome PRIVATE -flto=thin)
    else()
        target_compile_options(mx-welcome PRIVATE -flto=auto)
        target_link_options(mx-welcome PRIVATE -flto=auto)
    endif()
endif()

# Handle translations
qt6_add_translations(mx-welcome
    TS_FILES ${TRANSLATION_FILES}
    LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# Generate version.h file
# Use the full version string from changelog for the VERSION define
set(PROJECT_VERSION_FULL "${PROJECT_VERSION_FROM_CHANGELOG}")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_BINARY_DIR}/src/version.h"
    @ONLY
)

# Set target properties
set_target_properties(mx-welcome PROPERTIES
    OUTPUT_NAME "mx-welcome"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add build directory to include path for generated version.h
target_include_directories(mx-welcome PRIVATE
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS mx-welcome
    RUNTIME DESTINATION bin
)
