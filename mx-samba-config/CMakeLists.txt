# **********************************************************************
# * Copyright (C) 2017-2025 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-samba-config.
# *
# * mx-samba-config is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-samba-config is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-samba-config.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

cmake_minimum_required(VERSION 3.16)

# Option to use clang for testing builds - needs to be early so project() picks it up
option(USE_CLANG "Use clang compiler" OFF)
if(USE_CLANG)
    set(CMAKE_C_COMPILER clang CACHE STRING "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)
    message(STATUS "Using clang compiler")
endif()

# Get version from debian/changelog
execute_process(
    COMMAND dpkg-parsechangelog -SVersion
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION_FROM_CHANGELOG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE DPKG_RESULT
)

# Fallback to a safe default when changelog parsing is unavailable
if(NOT DPKG_RESULT EQUAL 0 OR PROJECT_VERSION_FROM_CHANGELOG STREQUAL "")
    message(WARNING "Falling back to version 0.0.0 (dpkg-parsechangelog not available or failed)")
    set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
endif()

project(mx-samba-config
    VERSION ${PROJECT_VERSION_FROM_CHANGELOG}
    DESCRIPTION "Samba configuration tool for MX Linux"
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

# Option to use clang for testing builds
if(USE_CLANG)
    set(CMAKE_CXX_COMPILER_ID "Clang")
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    LinguistTools
)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/editshare.cpp
    src/about.cpp
)

set(HEADERS
    src/mainwindow.h
    src/editshare.h
    src/about.h
)

set(UI_FILES
    src/mainwindow.ui
    src/editshare.ui
)

set(RESOURCE_FILES
    images.qrc
)

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")

# Create the executable
add_executable(mx-samba-config
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Link Qt6 libraries
target_link_libraries(mx-samba-config
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Set compiler flags
target_compile_options(mx-samba-config PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
    -Werror
)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(mx-samba-config PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(mx-samba-config PRIVATE -Werror=return-local-addr)
endif()

# Set compile definitions
target_compile_definitions(mx-samba-config PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(mx-samba-config PRIVATE NDEBUG)
    target_compile_options(mx-samba-config PRIVATE -O3)

    # Add LTO - different flags for different compilers
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
        target_compile_options(mx-samba-config PRIVATE -flto=thin)
        target_link_options(mx-samba-config PRIVATE -flto=thin)
    else()
        target_compile_options(mx-samba-config PRIVATE -flto=auto)
        target_link_options(mx-samba-config PRIVATE -flto=auto)
    endif()
endif()

# Handle translations
qt6_add_translations(mx-samba-config
    TS_FILES ${TRANSLATION_FILES}
    LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# Set target properties
set_target_properties(mx-samba-config PROPERTIES
    OUTPUT_NAME "mx-samba-config"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS mx-samba-config
    RUNTIME DESTINATION bin
)
