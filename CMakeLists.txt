cmake_minimum_required(VERSION 3.9)
enable_testing()

project(scopy VERSION 2.0.0 LANGUAGES CXX)
set(SCOPY_VERSION ${PROJECT_VERSION})

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

include(ScopyAbout)
configure_about(./resources/about)
file(GLOB SCOPY_RESOURCE_FILES gui/res/resources.qrc resources/aboutpage.qrc)
find_file(SCOPY_ICON_ICO icon.ico PATHS ${CMAKE_SOURCE_DIR}/gui/res)
message(STATUS "SCOPY_RESOURCE_FILES: " ${SCOPY_RESOURCE_FILES})

option(ENABLE_TRANSLATION "Enable translation" ON)
include(ScopyTranslation)

if(ENABLE_TRANSLATION)
	generate_translations()
	qt_add_resources(SCOPY_RESOURCES ${CMAKE_BINARY_DIR}/translations.qrc)
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

set(SCOPY_DLL_PATH ${CMAKE_INSTALL_LIBDIR})
set(SCOPY_PLUGIN_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/plugins/plugins)
set(SCOPY_PLUGIN_INSTALL_PATH ${CMAKE_INSTALL_FULL_DATADIR}/scopy-plugins)
set(SCOPY_TRANSLATION_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/translations)
set(SCOPY_TRANSLATION_INSTALL_PATH ${CMAKE_INSTALL_FULL_DATADIR}/translations)

add_subdirectory(common)
add_subdirectory(iioutil)
add_subdirectory(gui)
add_subdirectory(gr-util)
add_subdirectory(pluginbase)
add_subdirectory(core)
add_subdirectory(plugins)

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

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopy-core scopy-gui)

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
)

if(ENABLE_APPLICATION_BUNDLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME Scopy)
endif()

configure_file(resources/qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf COPYONLY)
configure_file(resources/scopy.desktop.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop @ONLY)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/icon_small.svg
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps RENAME scopy.svg
	)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/translations DESTINATION ${CMAKE_INSTALL_FULL_DATADIR})
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include(ScopyWindows)
	duplicate_target(${PROJECT_NAME} Scopy-console)
	set_target_properties(Scopy-console PROPERTIES WIN32_EXECUTABLE FALSE)
endif()
if(NOT ENABLE_APPLICATION_BUNDLE)
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(${PROJECT_NAME})
endif()
