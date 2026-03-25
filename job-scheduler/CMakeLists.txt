# **********************************************************************
# * Copyright (C) 2017-2025 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of custom-toolbox.
# *
# * custom-toolbox is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * custom-toolbox is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with custom-toolbox.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION_FROM_CHANGELOG "")

if(DEFINED PROJECT_VERSION_OVERRIDE AND NOT "${PROJECT_VERSION_OVERRIDE}" STREQUAL "")
    set(PROJECT_VERSION_FROM_CHANGELOG "${PROJECT_VERSION_OVERRIDE}")
    message(STATUS "Using version override: ${PROJECT_VERSION_FROM_CHANGELOG}")
else()
    # Try dpkg-parsechangelog first (Debian/Ubuntu)
    execute_process(
        COMMAND dpkg-parsechangelog -SVersion
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_VERSION_FROM_CHANGELOG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE DPKG_RESULT
        ERROR_QUIET
    )

    if(DPKG_RESULT EQUAL 0 AND NOT "${PROJECT_VERSION_FROM_CHANGELOG}" STREQUAL "")
        message(STATUS "Using version from debian/changelog: ${PROJECT_VERSION_FROM_CHANGELOG}")
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/debian/changelog)
        file(READ ${CMAKE_SOURCE_DIR}/debian/changelog PROJECT_CHANGELOG_CONTENT)
        string(REGEX MATCH "^[^\\(]*\\(([^)]+)\\)" _match "${PROJECT_CHANGELOG_CONTENT}")
        if(_match)
            set(PROJECT_VERSION_FROM_CHANGELOG "${CMAKE_MATCH_1}")
            message(STATUS "Using version parsed from debian/changelog: ${PROJECT_VERSION_FROM_CHANGELOG}")
        else()
            message(WARNING "Could not parse version from debian/changelog, defaulting to 0.0.0")
            set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
        endif()
    else()
        message(WARNING "Could not determine version automatically, defaulting to 0.0.0")
        set(PROJECT_VERSION_FROM_CHANGELOG "0.0.0")
    endif()
endif()

project(job-scheduler
    VERSION ${PROJECT_VERSION_FROM_CHANGELOG}
    DESCRIPTION "Job Scheduler - Graphical job scheduling utility for Linux"
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
option(USE_CLANG "Use clang compiler" OFF)
if(USE_CLANG)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_CXX_COMPILER_ID "Clang")
    message(STATUS "Using clang compiler")
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
    src/MainWindow.cpp
    src/about.cpp
    src/Crontab.cpp
    src/CronModel.cpp
    src/CronView.cpp
    src/CronTime.cpp
    src/ExecuteList.cpp
    src/ExecuteModel.cpp
    src/ExecuteView.cpp
    src/SaveDialog.cpp
    src/TCommandEdit.cpp
    src/TimeDialog.cpp
    src/VariableEdit.cpp
    src/VariableModel.cpp
    src/VariableView.cpp
)

set(HEADERS
    src/MainWindow.h
    src/about.h
    src/Clib.h
    src/Crontab.h
    src/CronModel.h
    src/CronView.h
    src/CronTime.h
    src/Execute.h
    src/ExecuteList.h
    src/ExecuteModel.h
    src/ExecuteView.h
    src/SaveDialog.h
    src/TCommandEdit.h
    src/TimeDialog.h
    src/VariableEdit.h
    src/VariableModel.h
    src/VariableView.h
)

set(UI_FILES
)

set(RESOURCE_FILES
    application.qrc
)

# Get all translation files
file(GLOB TRANSLATION_FILES "translations/*.ts")

# Create the executable
add_executable(job-scheduler
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Link Qt6 libraries
target_link_libraries(job-scheduler
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Set compiler flags
target_compile_options(job-scheduler PRIVATE
    -Wpedantic
    -pedantic
    -Werror=return-type
    -Werror=switch
    -Werror=uninitialized
    -Werror
)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
    target_compile_options(job-scheduler PRIVATE -Werror=return-stack-address)
else()
    target_compile_options(job-scheduler PRIVATE -Werror=return-local-addr)
endif()

# Set compile definitions
target_compile_definitions(job-scheduler PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    VERSION="${PROJECT_VERSION}"
)

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(job-scheduler PRIVATE NDEBUG)
    target_compile_options(job-scheduler PRIVATE -O3)

    # Add LTO - different flags for different compilers
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR USE_CLANG)
        target_compile_options(job-scheduler PRIVATE -flto=thin)
        target_link_options(job-scheduler PRIVATE -flto=thin)
    else()
        target_compile_options(job-scheduler PRIVATE -flto=auto)
        target_link_options(job-scheduler PRIVATE -flto=auto)
    endif()
endif()

# Handle translations
qt6_add_translations(job-scheduler
    TS_FILES ${TRANSLATION_FILES}
    LRELEASE_OPTIONS -compress -nounfinished -removeidentical -silent
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# Set target properties
set_target_properties(job-scheduler PROPERTIES
    OUTPUT_NAME "job-scheduler"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install target (required by Debian build system)
# Other files are handled by debian/install
install(TARGETS job-scheduler
    RUNTIME DESTINATION bin
)
