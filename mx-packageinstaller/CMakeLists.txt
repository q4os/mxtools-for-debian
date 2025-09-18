# **********************************************************************
# * Copyright (C) 2017-2025 MX Authors
# *
# * Authors: Adrian
# *          Dolphin_Oracle
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-packageinstaller.
# *
# * mx-packageinstaller is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-packageinstaller is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

cmake_minimum_required(VERSION 3.16)

# Allow selecting clang before project() to ensure proper toolchain detection
option(USE_CLANG "Use clang compiler" OFF)
if(USE_CLANG)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    message(STATUS "Using clang compiler")
endif()

# Version detection with override and fallback
set(PROJECT_VERSION_OVERRIDE "" CACHE STRING "Override project version (e.g. 1.2.3)")
set(PROJECT_VERSION_FROM_CHANGELOG "")
find_program(DPKG_PARSECHANGELOG_EXECUTABLE NAMES dpkg-parsechangelog)
if(PROJECT_VERSION_OVERRIDE)
    set(PROJECT_VERSION_FROM_CHANGELOG ${PROJECT_VERSION_OVERRIDE})
elseif(DPKG_PARSECHANGELOG_EXECUTABLE)
    execute_process(
        COMMAND ${DPKG_PARSECHANGELOG_EXECUTABLE} -SVersion
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_VERSION_FROM_CHANGELOG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE DPKG_RESULT
    )
    if(NOT DPKG_RESULT EQUAL 0 OR PROJECT_VERSION_FROM_CHANGELOG STREQUAL "")
        set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
        message(WARNING "Failed to get version from debian/changelog; defaulting to ${PROJECT_VERSION_FROM_CHANGELOG}")
    endif()
else()
    set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
    message(WARNING "dpkg-parsechangelog not found; defaulting to ${PROJECT_VERSION_FROM_CHANGELOG}")
endif()

project(mx-packageinstaller
    VERSION ${PROJECT_VERSION_FROM_CHANGELOG}
    DESCRIPTION "MX Package Installer - A tool for managing packages on MX Linux"
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

# Option to build tests
option(BUILD_TESTS "Build unit tests" OFF)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Xml
    LinguistTools
)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define source files
set(SOURCES
    src/log.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/checkableheaderview.cpp
    src/lockfile.cpp
    src/pmfiles.cpp
    src/versionnumber.cpp
    src/aptcache.cpp
    src/remotes.cpp
    src/about.cpp
    src/cmd.cpp
)

set(HEADERS
    src/log.h
    src/mainwindow.h
    src/checkableheaderview.h
    src/lockfile.h
    src/pmfiles.h
    src/versionnumber.h
    src/aptcache.h
    src/remotes.h
    src/about.h
    src/cmd.h
)

set(UI_FILES
    src/mainwindow.ui
)

set(RESOURCE_FILES
    src/images.qrc
)

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")

# Create the executable
add_executable(mx-packageinstaller
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Link Qt6 libraries
target_link_libraries(mx-packageinstaller
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

# Set compiler flags
target_compile_options(mx-packageinstaller PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
    -Werror
)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(mx-packageinstaller PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(mx-packageinstaller PRIVATE
        -Werror=return-local-addr
        # Suppress false positive LTO warnings with Qt6 QHash (GCC only)
        -Wno-alloc-size-larger-than
    )
endif()

# Set compile definitions
target_compile_definitions(mx-packageinstaller PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(mx-packageinstaller PRIVATE NDEBUG)
    target_compile_options(mx-packageinstaller PRIVATE -O3)

    # Add LTO - different flags for different compilers
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
        target_compile_options(mx-packageinstaller PRIVATE -flto=thin)
        target_link_options(mx-packageinstaller PRIVATE -flto=thin)
    else()
        target_compile_options(mx-packageinstaller PRIVATE -flto=auto)
        target_link_options(mx-packageinstaller PRIVATE
            -flto=auto
            # Suppress false positive LTO warnings with Qt6 QHash at link time (GCC only)
            -Wno-alloc-size-larger-than
        )
    endif()
endif()

# Handle translations
qt6_add_translations(mx-packageinstaller
    TS_FILES ${TRANSLATION_FILES}
    LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# Set target properties
set_target_properties(mx-packageinstaller PROPERTIES
    OUTPUT_NAME "mx-packageinstaller"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS mx-packageinstaller
    RUNTIME DESTINATION bin
)

# Add tests if requested
if(BUILD_TESTS)
    message(STATUS "Building with tests enabled")
    add_subdirectory(Testing)
endif()
