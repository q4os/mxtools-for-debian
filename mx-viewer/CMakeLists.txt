# **********************************************************************
# * Copyright (C) 2017-2026 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-viewer.
# *
# * mx-viewer is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-viewer is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-viewer.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

cmake_minimum_required(VERSION 3.16)

# Arch packaging supplies the version directly because dpkg is unavailable there.
if(PROJECT_VERSION_OVERRIDE)
    set(PROJECT_VERSION_FROM_CHANGELOG "${PROJECT_VERSION_OVERRIDE}")
else()
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
endif()

# Build with Clang. The compiler has to be chosen before project() enables the language; an explicit
# -DCMAKE_CXX_COMPILER wins.
option(USE_CLANG "Use clang compiler" OFF)
if(USE_CLANG AND NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()

project(mx-viewer
    VERSION ${PROJECT_VERSION_FROM_CHANGELOG}
    DESCRIPTION "MX Viewer - Lightweight web browser for MX Linux"
    LANGUAGES CXX
)
if(USE_CLANG AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(WARNING "USE_CLANG is ignored: this build directory already uses ${CMAKE_CXX_COMPILER}. "
                    "Configure a fresh build directory to build with Clang.")
endif()

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
    Network
    PrintSupport
    Sql
    WebEngineWidgets
    LinguistTools
)
find_package(Qt6Keychain REQUIRED)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/browsercontext.cpp
    src/bookmarks.cpp
    src/newtabpage.cpp
    src/historypage.cpp
    src/settingspage.cpp
    src/readermode.cpp
    src/dialogs.cpp
    src/windowchrome.cpp
    src/navigation.cpp
    src/iconutils.cpp
    src/pageutils.cpp
    src/webview.cpp
    src/tabwidget.cpp
    src/addressbar.cpp
    src/bookmarkbar.cpp
    src/findbar.cpp
    src/downloadwidget.cpp
    src/historystore.cpp
    src/singleinstance.cpp
)

set(HEADERS
    src/mainwindow.h
    src/mainwindowhelpers.h
    src/resizegrip.h
    src/tabmenubar.h
    src/webview.h
    src/tabwidget.h
    src/addressbar.h
    src/bookmarkbar.h
    src/findbar.h
    src/downloadwidget.h
    src/historystore.h
    src/singleinstance.h
)

set(UI_FILES
    src/downloadwidget.ui
)

set(RESOURCE_FILES
    images.qrc
)

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")
list(FILTER TRANSLATION_FILES EXCLUDE REGEX "/mx-viewer_en_US\\.ts$")

# Create the executable
add_executable(mx-viewer
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Link Qt6 libraries
target_link_libraries(mx-viewer
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Sql
    Qt6::WebEngineWidgets
    Qt6Keychain::Qt6Keychain
)

# Set compiler flags. Warnings fail the build only when asked (build.sh and CI do), so a newer
# compiler with new warnings cannot break distribution package builds.
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
target_compile_options(mx-viewer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
)
if(WARNINGS_AS_ERRORS)
    target_compile_options(mx-viewer PRIVATE -Werror)
endif()

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(mx-viewer PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(mx-viewer PRIVATE -Werror=return-local-addr)
endif()

# Set compile definitions
target_compile_definitions(mx-viewer PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)
# QKeychain::isAvailable() is new in qtkeychain 0.14; its version macro in the header is not kept up to date.
if(Qt6Keychain_VERSION VERSION_GREATER_EQUAL 0.14)
    target_compile_definitions(mx-viewer PRIVATE HAVE_KEYCHAIN_IS_AVAILABLE)
endif()

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(mx-viewer PRIVATE NDEBUG)
    target_compile_options(mx-viewer PRIVATE -O3)

    # Link-time optimization, with the flags CMake knows for the compiler in use.
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error LANGUAGES CXX)
    if(ipo_supported)
        set_property(TARGET mx-viewer PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "Link-time optimization not available: ${ipo_error}")
    endif()
endif()

# Handle translations
qt6_add_translations(mx-viewer
    TS_FILES ${TRANSLATION_FILES}
    LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# Set target properties
set_target_properties(mx-viewer PROPERTIES
    OUTPUT_NAME "mx-viewer"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add include directories for headers
target_include_directories(mx-viewer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS mx-viewer
    RUNTIME DESTINATION bin
)
