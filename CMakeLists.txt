#
# Copyright (c) 2024 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.9)
enable_testing()

project(scopy VERSION 2.1.0 LANGUAGES CXX)
set(SCOPY_RELEASE_PHASE -dev)
set(SCOPY_VERSION ${PROJECT_VERSION}${SCOPY_RELEASE_PHASE})

# set CMAKE_BUILD_TYPE if not set externally
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Default build type: RelWithDebInfo" FORCE)
endif()

# set default install path to /usr/local if not set externally
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "/usr/local")
endif()

find_program(CCACHE_FOUND ccache)
if(NOT DEFINED ENV{BUILD_HOST} AND CCACHE_FOUND)
	message(STATUS "Using ccache.")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
else()
	message(STATUS "Not using ccache.")
endif()

# Create the "compile_commands.json" file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(QT_MESSAGELOGCONTEXT)

include(GNUInstallDirs)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(KDDockWidgets QUIET)
if(KDDockWidgets_FOUND)
	message(STATUS "Using KDDockWidgets")
	add_compile_definitions(USE_KDDOCKWIDGETS)
else()
	message(STATUS "Not using KDDockWidgets")
endif()

if(Qt5Widgets_VERSION VERSION_LESS 5.15.2)
	message(FATAL_ERROR "Minimum supported Qt 5.15.2")
	return()
else()
	message(STATUS "Using Qt version: " ${Qt5Widgets_VERSION})
endif()

# message(QtVersion: ${QT_VERSION_MAJOR}:${QT_VERSION_MINOR})

file(GLOB SRC_LIST *.cpp *.cc)
file(GLOB HEADER_LIST *.h *.hpp)
file(GLOB UI_LIST *.ui)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

include(ScopyWhatsNew)
set(FOLDER ${CMAKE_SOURCE_DIR}/resources/whatsnew/)
set(RESOURCE_FILE ${CMAKE_SOURCE_DIR}/resources/whatsnew/whatsnew.qrc)
generate_whats_new("${RESOURCE_FILE}" "${FOLDER}")
file(GLOB WHATS_NEW_RESOURCES resources/whatsnew/whatsnew.qrc)
qt_add_resources(SCOPY_RESOURCES ${WHATS_NEW_RESOURCES})

include(ScopyAbout)
configure_about(./resources/about)
file(GLOB SCOPY_RESOURCE_FILES gui/res/resources.qrc resources/aboutpage.qrc)
find_file(SCOPY_ICON_ICO icon.ico PATHS ${CMAKE_SOURCE_DIR}/gui/res)
message(STATUS "SCOPY_RESOURCE_FILES: " ${SCOPY_RESOURCE_FILES})

include(ScopyStyle)
generate_style(
	"--core" ${CMAKE_CURRENT_SOURCE_DIR}/gui/style ${CMAKE_CURRENT_SOURCE_DIR}/gui/include/gui ${CMAKE_BINARY_DIR}
)

option(ENABLE_TRANSLATION "Enable translation" ON)
include(ScopyTranslation)

if(ENABLE_TRANSLATION)
	generate_translations(${CMAKE_BINARY_DIR}/translations)
	qt_add_resources(SCOPY_RESOURCES ${CMAKE_BINARY_DIR}/translations.qrc)
endif()

if(DEFINED ENV{APPIMAGE})
	add_compile_definitions(__appimage__)
endif()

qt_add_resources(SCOPY_RESOURCES ${SCOPY_RESOURCE_FILES})

if(WIN32)
	# Set .exe properties
	string(REPLACE "." "," SCOPY_FILEVERSION ${PROJECT_VERSION})
	set(SCOPY_PRODUCTVERSION_STR ${PROJECT_VERSION})
	string(TIMESTAMP BUILD_YEAR "%Y")
	set(SCOPY_WIN32_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources/properties.rc)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/properties.rc.cmakein ${SCOPY_WIN32_RESOURCES} @ONLY)
endif()

find_path(
	IIO_INCLUDE_DIRS
	NAMES iio.h
	HINTS ${CMAKE_INSTALL_PREFIX}/include
	      /include
	      /usr/include
	      /usr/local/include
	      /opt/local/include
	      REQUIRED
)

find_library(
	IIO_LIBRARIES
	NAMES iio libiio
	HINTS ${CMAKE_INSTALL_PREFIX}/lib
	      /usr/lib
	      /usr/lib64
	      /usr/local/lib
	      /usr/local/lib64
	      /opt/local/lib
	      /opt/local/lib64
	      REQUIRED
)
message("IIO LIBRARIES: " ${IIO_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(SCOPY_DLL_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/Frameworks)
	set(SCOPY_TRANSLATION_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/translations)
	set(SCOPY_STYLE_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/style)
	set(SCOPY_PACKAGE_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/packages)
else()
	set(SCOPY_DLL_BUILD_PATH ${CMAKE_BINARY_DIR})
	set(SCOPY_TRANSLATION_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/translations)
	set(SCOPY_STYLE_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/style)
	set(SCOPY_PACKAGE_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/packages)
endif()

set(SCOPY_DLL_INSTALL_PATH ${CMAKE_INSTALL_FULL_LIBDIR})
set(SCOPY_TRANSLATION_INSTALL_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/scopy)
set(SCOPY_STYLE_INSTALL_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/scopy)
set(SCOPY_PDK pdk)
set(SCOPY_PACKAGE_INSTALL_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/scopy/packages)

option(ENABLE_TESTING "Enable unit tests" ON)
if(ENABLE_TESTING)
	message(STATUS "Unit tests enabled")
	add_subdirectory(tests)
endif()

option(ENABLE_APPLICATION_BUNDLE "Enable application bundle for OSX" OFF)
if(APPLE)
	set(ENABLE_APPLICATION_BUNDLE ON)
	include(ScopyMacOS)
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCOPY_DLL_BUILD_PATH})
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_DLL_BUILD_PATH})
endif()

add_subdirectory(common)
add_subdirectory(iioutil)
add_subdirectory(gui)
add_subdirectory(gr-util)
add_subdirectory(pluginbase)
add_subdirectory(pkg-manager)
add_subdirectory(core)
add_subdirectory(iio-widgets)
add_subdirectory(packages)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS")
endif()

set(SCOPY_DEPENDENCIES ${IIO_LIBRARIES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(
		${PROJECT_NAME}
		MANUAL_FINALIZATION
		${PROJECT_SOURCES}
		${SCOPY_RESOURCES}
		${SCOPY_DEPENDENCIES}
		${SCOPY_WIN32_RESOURCES}
	)
	# Define target properties for Android with Qt 6 as: set_property(TARGET tool_launcher APPEND PROPERTY
	# QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android) For more information, see
	# https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
	if(ANDROID)
		add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${SCOPY_RESOURCES} ${SCOPY_DEPENDENCIES})
		# Define properties for Android with Qt 5 after find_package() calls as: set(ANDROID_PACKAGE_SOURCE_DIR
		# "${CMAKE_CURRENT_SOURCE_DIR}/android")
	else()
		add_executable(
			${PROJECT_NAME} WIN32
			${OSX_BUNDLE}
			${PROJECT_SOURCES}
			${SCOPY_RESOURCES}
			${SCOPY_DEPENDENCIES}
			${EXTRA_BUNDLE_FILES}
			${SCOPY_WIN32_RESOURCES}
		)
	endif()
endif()

find_package(ECM REQUIRED NO_MODULE)
message(STATUS "ECM found at: ${ECM_DIR}")

find_package(KF5Archive REQUIRED)
if(KF5Archive_FOUND)
	message(STATUS "Using KF5Archive")
else()
	message(STATUS "Not using KF5Archive")
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-core)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopy-core scopy-gui)

if(KF5Archive_FOUND)
	target_link_libraries(${PROJECT_NAME} PUBLIC KF5::Archive)
endif()

# Link optional libraries
if(KDDockWidgets_FOUND)
	target_link_libraries(${PROJECT_NAME} PUBLIC KDAB::kddockwidgets)
endif()

# Compiler options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

# List of warnings to be treated as errors
target_compile_options(
	${PROJECT_NAME} PUBLIC -Werror=return-type -Werror=uninitialized -Werror=init-self -Werror=switch
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/apple/Info.plist.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Info.plist COPYONLY)
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER scopy
		   MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
		   MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
		   MACOSX_BUNDLE TRUE
		   MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
		   WIN32_EXECUTABLE TRUE
		   ENABLE_EXPORTS ON # equiv to -rdynamic
)

if(ENABLE_APPLICATION_BUNDLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME Scopy)
endif()

configure_file(resources/qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf COPYONLY)
configure_file(resources/scopy.desktop.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop @ONLY)

if(DEFINED ENV{APPIMAGE})
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ci/general/scopy.desktop
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications
	)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/scopy.png
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/512x512/apps RENAME scopy.png
	)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/icon_small.svg
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps RENAME scopy.svg
	)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/icon_small.svg
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps RENAME scopy.svg
	)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include(ScopyWindows)
	duplicate_target(${PROJECT_NAME} Scopy-console)
	set_target_properties(Scopy-console PROPERTIES WIN32_EXECUTABLE FALSE)
	install(TARGETS Scopy-console DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt.conf DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
endif()
if(NOT ENABLE_APPLICATION_BUNDLE)
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	)
else()
	install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
endif()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/translations DESTINATION ${SCOPY_TRANSLATION_INSTALL_PATH})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/style DESTINATION ${SCOPY_STYLE_INSTALL_PATH})

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(${PROJECT_NAME})
endif()

# make uninstall
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
	TARGET "uninstall"
	POST_BUILD
	COMMENT "Uninstall files with install_manifest.txt"
	COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in install_manifest.txt to be uninstalled!
)

# CPack config
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}")

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_SET_DESTDIR ON)
set(CPACK_COMPONENTS_ALL ${SCOPY_PDK})
include(CPack)
