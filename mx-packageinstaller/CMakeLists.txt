# **********************************************************************
# * Copyright (C) 2017-2026 MX Authors
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

# Version detection with override and fallback. Tries, in order: an explicit
# override, Debian's debian/changelog (via dpkg-parsechangelog, when available --
# the Debian/MX packaging path), then the root PKGBUILD's pkgver= (the Arch
# packaging path, including its `pkgver=${PKGVER:-<default>}` override syntax).
# Whichever tool is actually present determines which path fires; this doesn't
# need to know PACKAGE_BACKEND (selected further below, after project()).
set(PROJECT_VERSION_OVERRIDE "" CACHE STRING "Override project version (e.g. 1.2.3)")
set(PROJECT_VERSION_FROM_CHANGELOG "")
find_program(DPKG_PARSECHANGELOG_EXECUTABLE NAMES dpkg-parsechangelog)
if(PROJECT_VERSION_OVERRIDE)
    set(PROJECT_VERSION_FROM_CHANGELOG ${PROJECT_VERSION_OVERRIDE})
elseif(DPKG_PARSECHANGELOG_EXECUTABLE AND EXISTS ${CMAKE_SOURCE_DIR}/debian/changelog)
    execute_process(
        COMMAND ${DPKG_PARSECHANGELOG_EXECUTABLE} -SVersion
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_VERSION_FROM_CHANGELOG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE DPKG_RESULT
    )
    if(NOT DPKG_RESULT EQUAL 0 OR PROJECT_VERSION_FROM_CHANGELOG STREQUAL "")
        set(PROJECT_VERSION_FROM_CHANGELOG "")
        message(WARNING "Failed to get version from debian/changelog")
    endif()
endif()

if(PROJECT_VERSION_FROM_CHANGELOG STREQUAL "" AND EXISTS ${CMAKE_SOURCE_DIR}/PKGBUILD)
    file(READ ${CMAKE_SOURCE_DIR}/PKGBUILD PROJECT_PKGBUILD_CONTENT)
    string(REGEX MATCH "(^|[\r\n])[ \t]*pkgver=([^ \t\r\n]+)" _pkgver_match "${PROJECT_PKGBUILD_CONTENT}")
    set(PROJECT_PKGBUILD_VER "${CMAKE_MATCH_2}")
    if(_pkgver_match)
        # Support pkgver=${PKGVER:-<version>} default syntax
        string(REGEX MATCH "^\\$\\{PKGVER:-([^}]+)\\}$" _pkgver_default_match "${PROJECT_PKGBUILD_VER}")
        if(_pkgver_default_match)
            set(PROJECT_PKGBUILD_VER "${CMAKE_MATCH_1}")
        endif()
        set(PROJECT_VERSION_FROM_CHANGELOG "${PROJECT_PKGBUILD_VER}")
        message(STATUS "Using version from PKGBUILD: ${PROJECT_VERSION_FROM_CHANGELOG}")
    endif()
endif()

if(PROJECT_VERSION_FROM_CHANGELOG STREQUAL "")
    set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
    message(WARNING "Could not determine version from debian/changelog or PKGBUILD; defaulting to ${PROJECT_VERSION_FROM_CHANGELOG}")
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

# Which package-manager backend to build against. Auto-detected from what's on the
# build machine, but can be overridden explicitly (e.g. cross-building/packaging).
if(NOT DEFINED PACKAGE_BACKEND)
    find_program(APT_GET_EXECUTABLE apt-get)
    find_program(PACMAN_EXECUTABLE pacman)
    if(APT_GET_EXECUTABLE)
        set(PACKAGE_BACKEND_DEFAULT "apt")
    elseif(PACMAN_EXECUTABLE)
        set(PACKAGE_BACKEND_DEFAULT "pacman")
    else()
        set(PACKAGE_BACKEND_DEFAULT "apt")
        message(WARNING "Neither apt-get nor pacman found; defaulting PACKAGE_BACKEND to apt")
    endif()
else()
    set(PACKAGE_BACKEND_DEFAULT "${PACKAGE_BACKEND}")
endif()
set(PACKAGE_BACKEND "${PACKAGE_BACKEND_DEFAULT}" CACHE STRING "Package-manager backend to build (apt or pacman)")
set_property(CACHE PACKAGE_BACKEND PROPERTY STRINGS apt pacman)
if(NOT PACKAGE_BACKEND STREQUAL "apt" AND NOT PACKAGE_BACKEND STREQUAL "pacman")
    message(FATAL_ERROR "PACKAGE_BACKEND must be 'apt' or 'pacman', got: ${PACKAGE_BACKEND}")
endif()
message(STATUS "Building PACKAGE_BACKEND=${PACKAGE_BACKEND}")

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

# Absolute directory where the privileged helper binary is installed. Defaults differ
# by backend to match each distro's existing packaging convention (Debian/MX uses a
# dedicated libexec dir; the pre-existing Arch package/AUR PKGBUILD installs to
# /usr/lib/mx-packageinstaller) -- override explicitly if that's ever not what you want.
if(PACKAGE_BACKEND STREQUAL "pacman")
    set(HELPER_INSTALL_DIR_DEFAULT "/usr/lib/mx-packageinstaller")
else()
    set(HELPER_INSTALL_DIR_DEFAULT "/usr/libexec/mx-packageinstaller")
endif()
set(HELPER_INSTALL_DIR "${HELPER_INSTALL_DIR_DEFAULT}" CACHE PATH
    "Absolute install directory for the privileged helper binary")
if(NOT IS_ABSOLUTE "${HELPER_INSTALL_DIR}")
    message(FATAL_ERROR "HELPER_INSTALL_DIR must be an absolute path, got: ${HELPER_INSTALL_DIR}")
endif()
set(HELPER_PATH "${HELPER_INSTALL_DIR}/helper")

configure_file(
    scripts/org.mxlinux.pkexec.mxpi-helper.policy.in
    ${CMAKE_BINARY_DIR}/org.mxlinux.pkexec.mxpi-helper.policy
    @ONLY
)

# Define source files
set(SOURCES
    src/log.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/checkableheaderview.cpp
    src/pmfiles.cpp
    src/versionnumber.cpp
    src/remotes.cpp
    src/about.cpp
    src/cmd.cpp
    src/models/packagemodel.cpp
    src/models/packagefilterproxy.cpp
    src/models/flatpakmodel.cpp
    src/models/flatpakfilterproxy.cpp
    src/models/snapmodel.cpp
    src/models/snapfilterproxy.cpp
    src/models/popularmodel.cpp
    src/models/popularfilterproxy.cpp
)

set(HELPER_SOURCES
    src/helper.cpp
    src/helperengine.cpp
)

set(HEADERS
    src/log.h
    src/mainwindow.h
    src/checkableheaderview.h
    src/lockfile.h
    src/pmfiles.h
    src/versionnumber.h
    src/remotes.h
    src/about.h
    src/cmd.h
    src/outputrender.h
    src/sizeutils.h
    src/packagestatus.h
    src/packageinfo.h
    src/packagebackend.h
    src/models/packagemodel.h
    src/models/packagefilterproxy.h
    src/models/flatpakmodel.h
    src/models/flatpakfilterproxy.h
    src/models/snapmodel.h
    src/models/snapfilterproxy.h
    src/models/popularmodel.h
    src/models/popularfilterproxy.h
)

# Backend-specific sources. lockfile.{h,cpp} and packagebackend.h share their headers
# above; only one implementation of each is compiled in, matching each package
# manager's own semantics (apt: dpkg frontend + raw lock check, dpkg-query/apt-mark/
# apt-get; pacman: direct lockf() probe, pacman -Qi/-D/-Qtdq).
if(PACKAGE_BACKEND STREQUAL "pacman")
    list(APPEND SOURCES src/lockfile_pacman.cpp src/packagebackend_pacman.cpp)
else()
    list(APPEND SOURCES src/lockfile_apt.cpp src/packagebackend_apt.cpp)
    # Hand-parses raw Debian archive Packages files -- no pacman equivalent.
    list(APPEND SOURCES src/aptcache.cpp)
    list(APPEND HEADERS src/aptcache.h)
endif()

set(UI_FILES
    src/mainwindow.ui
)

set(RESOURCE_FILES
    src/images.qrc
)

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")
list(FILTER TRANSLATION_FILES EXCLUDE REGEX "/mx-packageinstaller_en_US\\.ts$")

# Create the executable
add_executable(mx-packageinstaller
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

add_executable(helper
    ${HELPER_SOURCES}
)

# Link Qt6 libraries
target_link_libraries(mx-packageinstaller
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

if(PACKAGE_BACKEND STREQUAL "pacman")
    # packagebackend_pacman.cpp dlopen()s libalpm at runtime (see its
    # compareVersions() for why: it wants alpm_pkg_vercmp() without requiring
    # libalpm-dev's headers/pkg-config file at build time). dlopen/dlsym
    # themselves come from libdl pre-glibc-2.34 and from libc itself after;
    # CMAKE_DL_LIBS resolves to whichever is correct for the toolchain.
    target_link_libraries(mx-packageinstaller ${CMAKE_DL_LIBS})
endif()

target_link_libraries(helper
    Qt6::Core
    Qt6::Xml
    util
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

target_compile_options(helper PRIVATE
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
    target_compile_options(helper PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(mx-packageinstaller PRIVATE
        -Werror=return-local-addr
        # Suppress false positive LTO warnings with Qt6 QHash (GCC only)
        -Wno-alloc-size-larger-than
    )
    target_compile_options(helper PRIVATE
        -Werror=return-local-addr
        -Wno-alloc-size-larger-than
    )
endif()

# Set compile definitions
target_compile_definitions(mx-packageinstaller PRIVATE
    HELPER_PATH="${HELPER_PATH}"
)

target_compile_definitions(mx-packageinstaller PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)

target_compile_definitions(helper PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)

if(PACKAGE_BACKEND STREQUAL "pacman")
    target_compile_definitions(mx-packageinstaller PRIVATE PACKAGE_BACKEND_PACMAN)
    target_compile_definitions(helper PRIVATE PACKAGE_BACKEND_PACMAN)
else()
    target_compile_definitions(mx-packageinstaller PRIVATE PACKAGE_BACKEND_APT)
    target_compile_definitions(helper PRIVATE PACKAGE_BACKEND_APT)
endif()

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

set_target_properties(helper PROPERTIES
    OUTPUT_NAME "helper"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS mx-packageinstaller
    RUNTIME DESTINATION bin
)

install(TARGETS helper
    RUNTIME DESTINATION ${HELPER_INSTALL_DIR}
)

# Add tests if requested
if(BUILD_TESTS)
    message(STATUS "Building with tests enabled")
    enable_testing()
    add_subdirectory(Testing)
endif()
