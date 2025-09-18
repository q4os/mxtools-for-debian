cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(build.cmake)

project(quick-system-info-gui
	VERSION 3.0
	DESCRIPTION "Quick System Info - A utility to show quick-system-info (inxi, etc...) in a gui"
	LANGUAGES CXX
)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_standard_project_setup()

qt_add_executable(quick-system-info-gui
	main.cpp
	about.cpp about.h
	mainwindow.cpp mainwindow.h mainwindow.ui
	qsi_resource.qrc
)

target_link_libraries(quick-system-info-gui PRIVATE
	Qt::Core
	Qt::Gui
	Qt::Widgets
	archive
)

build_compilation_setup(quick-system-info-gui)
build_version_definition(quick-system-info-gui VERSION)
build_translation_files(quick-system-info-gui translations)

include(GNUInstallDirs)
install(TARGETS quick-system-info-gui
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
