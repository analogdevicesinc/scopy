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

cmake_minimum_required(VERSION 3.16)
enable_testing()

project(scopy VERSION 2.3.0 LANGUAGES CXX)
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(QT_MESSAGELOGCONTEXT)

include(GNUInstallDirs)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(KDDockWidgets-qt6 QUIET)
if(KDDockWidgets-qt6_FOUND)
	message(STATUS "Using KDDockWidgets")
	add_compile_definitions(USE_KDDOCKWIDGETS)
else()
	message(STATUS "Not using KDDockWidgets")
endif()

message(STATUS "Using Qt version: ${QT_VERSION}")

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

# libiio API Scopy is built against (see device-controller/CMakeLists.txt). Declared here because the v0 lookup below
# depends on it. Only v0 builds need the flat <iio.h> and the v0 library; v1 builds use libiio solely through the
# device-controller backend, which finds libiio v1 itself.
set(SCOPY_LIBIIO_VERSION "1" CACHE STRING "libiio API used by the device-controller backend (0 or 1)")
set_property(CACHE SCOPY_LIBIIO_VERSION PROPERTY STRINGS 0 1)

if(SCOPY_LIBIIO_VERSION STREQUAL "0")
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
	message(STATUS "IIO LIBRARIES: " ${IIO_LIBRARIES})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(SCOPY_DLL_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/Frameworks)
	set(SCOPY_TRANSLATION_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/Resources/translations)
	set(SCOPY_STYLE_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/Resources/style)
	set(SCOPY_PACKAGE_BUILD_PATH ${CMAKE_BINARY_DIR}/Scopy.app/Contents/Resources/packages)
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

find_package(QCoro6 REQUIRED COMPONENTS Core Coro)
# Set necessary compiler flags to enable coroutine support
qcoro_enable_coroutines()

add_subdirectory(device-controller)
add_subdirectory(common)
add_subdirectory(iioutil)
add_subdirectory(gui)
# Default the gnuradio consumer OFF: gr-util below hard-requires the gnuradio stack, which the CI dependency images no
# longer ship. It must be DEFINED and false for the skip to trigger, so set it here rather than relying on the caller.
# Same idiom as packages/CMakeLists.txt.
if(NOT DEFINED ENABLE_PLUGIN_ADC)
	set(ENABLE_PLUGIN_ADC OFF CACHE BOOL "Enable adc plugin" FORCE)
endif()

# Debugger plugin is not migrated to the device-controller on this branch. Force it OFF unless the caller opts in, same
# idiom as ENABLE_PLUGIN_ADC above.
if(NOT DEFINED ENABLE_PLUGIN_DEBUGGER)
	set(ENABLE_PLUGIN_DEBUGGER OFF CACHE BOOL "Enable debugger plugin" FORCE)
endif()

# gr-util hard-requires the gnuradio stack and is consumed only by the adc plugin. Skip it when that flag is explicitly
# disabled so gnuradio can be dropped. (add_subdirectory(packages) synthesizes the flag later, but the default above -
# or a value passed on the command line - already puts it in the cache by this point.)
if(DEFINED ENABLE_PLUGIN_ADC AND NOT ENABLE_PLUGIN_ADC)
	message(STATUS "Skipping gr-util (ADC plugin disabled)")
else()
	add_subdirectory(gr-util)
endif()
add_subdirectory(pluginbase)
add_subdirectory(pkg-manager)
add_subdirectory(core)
add_subdirectory(iio-widgets)
add_subdirectory(packages)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS")
endif()

if(ENABLE_SCOPYJS)
	message(STATUS "JS tests enabled - copying js/ to build directory")
	file(COPY ${CMAKE_SOURCE_DIR}/js/ DESTINATION ${CMAKE_BINARY_DIR}/js)
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
	# Android packaging properties, consumed by androiddeployqt during target finalization.
	# https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
	#
	# Note QT_ANDROID_ABIS is deliberately NOT set: a multi-ABI build compiles the secondary ABIs as ExternalProject
	# sub-builds, and CMake cache variables passed on the command line are not forwarded to them unless named in
	# QT_ANDROID_MULTI_ABI_FORWARD_VARS. Scopy is configured almost entirely through -D flags, so each ABI is built
	# as its own single-ABI build instead, selecting the ABI via the Qt kit used to configure (android_x86_64 /
	# android_arm64_v8a).
	if(ANDROID)
		# androiddeployqt copies this directory over Qt's own templates, so it is assembled in the build tree
		# rather than used from the source tree: the data assets below are generated, and android/ must stay
		# clean.
		set(SCOPY_ANDROID_PACKAGE_DIR ${CMAKE_BINARY_DIR}/android-package)
		# versionCode must be a monotonically increasing integer.
		math(EXPR SCOPY_ANDROID_VERSION_CODE
		     "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}"
		)
		# min 28 is a hard floor in Qt 6.8's androiddeployqt; target 34 is its default and stays within what the
		# bundled Android Gradle Plugin supports.
		set_target_properties(
			${PROJECT_NAME}
			PROPERTIES QT_ANDROID_PACKAGE_SOURCE_DIR ${SCOPY_ANDROID_PACKAGE_DIR}
				   QT_ANDROID_MIN_SDK_VERSION 28
				   QT_ANDROID_TARGET_SDK_VERSION 34
				   QT_ANDROID_PACKAGE_NAME "org.adi.scopy"
				   QT_ANDROID_VERSION_NAME "${SCOPY_VERSION}"
				   QT_ANDROID_VERSION_CODE "${SCOPY_ANDROID_VERSION_CODE}"
		)
		# Android only loads lib*.so from the APK's lib/<abi>/ dir, flat and unversioned, and cannot dlopen from
		# assets:/ or a subdirectory. So every native library a plugin links has to end up in that dir. The
		# third-party part of the set depends on the dependency staging layout, so it is supplied by the build
		# (ci/android/android_deps_build.sh collect_extra_libs) rather than hardcoded.
		if(SCOPY_ANDROID_EXTRA_LIBS)
			# Beyond this list, androiddeployqt fills lib/<abi>/ from the application binary's recursive
			# dependency closure. Scopy libraries that only plugins link are in neither place, so they never
			# reach the APK and every plugin needing them fails to dlopen. Append them explicitly.
			list(APPEND SCOPY_ANDROID_EXTRA_LIBS "${SCOPY_DLL_BUILD_PATH}/libscopy-iio-widgets.so")
			# rfpowermeter links the datalogger plugin, which ships as an asset rather than in lib/<abi>/,
			# and the INSTALL_RPATH it sets does not apply to the build-tree copy that gets staged. Bundling
			# it here resolves that dependency without relying on datalogger happening to be dlopen'd first.
			list(APPEND SCOPY_ANDROID_EXTRA_LIBS
			     "${SCOPY_PACKAGE_BUILD_PATH}/generic-plugins/plugins/libscopy-datalogger.so"
			)
			set_target_properties(
				${PROJECT_NAME} PROPERTIES QT_ANDROID_EXTRA_LIBS "${SCOPY_ANDROID_EXTRA_LIBS}"
			)
		else()
			message(
				WARNING
					"SCOPY_ANDROID_EXTRA_LIBS is empty: the APK will ship without plugin/dependency "
					"native libraries, so the app will start with no plugins and no style."
			)
		endif()
	endif()
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

if(APPLE)
	setup_macos_bundle_resources(${PROJECT_NAME})
endif()

find_package(ECM REQUIRED NO_MODULE)
message(STATUS "ECM found at: ${ECM_DIR}")

find_package(KF6Archive REQUIRED)
if(KF6Archive_FOUND)
	message(STATUS "Using KF6Archive")
else()
	message(STATUS "Not using KF6Archive")
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-core)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopy-core scopy-gui)

if(KF6Archive_FOUND)
	target_link_libraries(${PROJECT_NAME} PUBLIC KF6::Archive)
endif()

if(ANDROID)
	# JNI headers for the QJniObject call in core/src/application_restarter.cpp, and the native libraries the Qt
	# Widgets stack expects to be linked explicitly on Android. libunwind is required because the NDK does not link
	# it implicitly: https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md#Unwinding
	find_package(JNI)
	if(JNI_FOUND)
		target_include_directories(${PROJECT_NAME} PRIVATE ${JNI_INCLUDE_DIRS})
	endif()
	target_link_libraries(${PROJECT_NAME} PRIVATE EGL GLESv2 unwind)
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
	if(ANDROID)
		# Scopy resolves style/translations/packages from $APPDATA on Android, which ScopyApplication.java
		# points at the app cache dir and Helpers.java fills from the APK assets on first run. So those three
		# trees have to ship as assets.
		#
		# The packages/ tree is copied complete, plugin .so files included: PkgManager discovers plugins by
		# listing <pkg>/plugins/ and the manifest does not name them, and those extracted copies are what
		# PluginManager dlopen's. lib/<abi>/ carries no plugin at all, only the libraries the plugins link
		# against - which is why QT_ANDROID_EXTRA_LIBS above has to name the ones nothing else pulls in.
		add_custom_target(
			scopy_android_package ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/android
				${SCOPY_ANDROID_PACKAGE_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SCOPY_STYLE_BUILD_PATH}
				${SCOPY_ANDROID_PACKAGE_DIR}/assets/style
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SCOPY_TRANSLATION_BUILD_PATH}
				${SCOPY_ANDROID_PACKAGE_DIR}/assets/translations
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SCOPY_PACKAGE_BUILD_PATH}
				${SCOPY_ANDROID_PACKAGE_DIR}/assets/packages
			COMMENT "Staging Android package source dir and data assets"
		)
		# Plugins are loaded at runtime, so nothing links them and the build system has no reason to finish them
		# before the copy above. Order it explicitly against every package/plugin target (recorded by
		# PackageUtils.cmake), otherwise a clean build can stage an incomplete packages/ tree.
		get_property(SCOPY_PACKAGE_TARGETS GLOBAL PROPERTY SCOPY_PACKAGE_TARGETS)
		if(SCOPY_PACKAGE_TARGETS)
			add_dependencies(scopy_android_package ${SCOPY_PACKAGE_TARGETS})
		endif()
		add_dependencies(${PROJECT_NAME}_make_apk scopy_android_package)
	endif()
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
