# Copyright (c) 2019 Analog Devices Inc.
#
# This file is part of Scopy
# (see http://www.github.com/analogdevicesinc/scopy).
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
# along with this program. If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.5)
project(scopy LANGUAGES CXX VERSION 1.2.0)

set(Python_ADDITIONAL_VERSIONS 3)
FIND_PACKAGE(PythonInterp REQUIRED)
set(PYTHON_VERSION python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})


function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
	if (ARGV0)
	    unset(MATCHED)
	    string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
	    if (NOT MATCHED)
		continue()
	    endif()
	endif()
	message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()


function(create_build_info_html_pages)

endfunction()
# Get the GIT hash of the latest commit
include(FindGit OPTIONAL)
if (GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE SCOPY_GIT_REPO
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE SCOPY_VERSION_GIT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if (NOT SCOPY_VERSION_GIT)
	set(SCOPY_VERSION_GIT v${PROJECT_VERSION})
endif()

# Set .exe properties
if (WIN32)
	string(REPLACE "." "," SCOPY_FILEVERSION ${PROJECT_VERSION})
	set(SCOPY_PRODUCTVERSION_STR ${PROJECT_VERSION})
	string(TIMESTAMP BUILD_YEAR "%Y")
endif()

if (MSVC)
	# http://www.suodenjoki.dk/us/archive/2010/min-max.htm
	add_definitions(-DNOMINMAX)

	# http://www.qtcentre.org/threads/32028-unresolved-external-symbol-quot-public-static-struct-QMetaObject-const-QwtPlot-staticMe
	add_definitions(-DQWT_DLL)
endif (MSVC)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
find_package(Qt5Concurrent REQUIRED)

FILE(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/resources/translations/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR})

# Creates translation .ts files from ${CMAKE_SOURCE_DIR}
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# Generate .qm files from the .ts files
qt5_add_translation(QM_FILES ${TS_FILES})

set(TRANSLATIONS)
foreach(file ${TS_FILES})
	get_filename_component(file_name ${file} NAME_WE)
	set(TRANSLATIONS "${TRANSLATIONS}\n<file>${file_name}.qm</file>")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/resources/translations.qrc
	${CMAKE_BINARY_DIR}/translations.qrc
	@ONLY)

qt5_add_resources(TRANSLATION_RESOURCES ${CMAKE_BINARY_DIR}/translations.qrc)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)  - Can't figure out how to configure it to look under ui/
#                         for the .ui file. Use qt5_wrap_ui instead.

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DQT_NO_DEBUG_OUTPUT=1)
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
if (NOT "${OUT_CONSTEXPR}" STREQUAL "-1")
	add_definitions(-DHAS_CONSTEXPR=1)
endif()


option(BREAKPAD_HANDLER "Build with breakpad exception handler " OFF)
message(STATUS BREAKPAD_HANDLER - ${BREAKPAD_HANDLER})
if (${BREAKPAD_HANDLER})
	message("-- Building with breakpad crash handler")
	set(BREAKPAD_HANDLER_BOOL 1)
	find_library(BREAKPAD_LIBRARIES NAMES breakpad)
	find_library(BREAKPADCLIENT_LIBRARIES NAMES breakpad_client)
	find_path(BREAKPAD_INCLUDE_DIRS breakpad)
else()
	message("-- Building without breakpad crash handler")
	set(BREAKPAD_HANDLER_BOOL 0)
	SET(BREAKPAD_LIBRARIES "")
	SET(BREAKPAD_INCLUDE_DIRS "")
	SET(BREAKPADCLIENT_LIBRARIES "")

endif()

find_package(Qwt REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5UiTools REQUIRED)
find_package(Qt5Xml REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
find_package(Gnuradio "3.8" REQUIRED COMPONENTS runtime analog blocks fft filter volk pmt )

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake)
find_package(gnuradio-iio REQUIRED PATH_SUFFIXES iio)
find_package(gnuradio-scopy REQUIRED PATH_SUFFIXES scopy)
find_package(gnuradio-m2k REQUIRED PATH_SUFFIXES m2k)

add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost COMPONENTS system filesystem thread chrono REQUIRED)

find_package(libm2k REQUIRED)

if (ENABLE_MATIO)
	message("-- Building with MATLAB support for SignalGenerator")
	find_library(MATIO_LIBRARIES REQUIRED NAMES matio)
	add_definitions(-DMATLAB_SUPPORT_SIGGEN)
endif()

find_library(QWTPOLAR_LIBRARIES qwtpolar)
find_path(QWTPOLAR_INCLUDE_DIRS qwt_polar_plot.h PATH_SUFFIXES qwt)
if (QWTPOLAR_LIBRARIES)
	message(STATUS "QwtPolar libraries found - ${QWTPOLAR_LIBRARIES}")
else()
	message(STATUS "QwtPolar libraries not found - assuming they are built in Qwt")
	set(QWTPOLAR_LIBRARIES "")
endif()
if (NOT QWTPOLAR_INCLUDE_DIRS)
	message(SEND_ERROR "QwtPolar includes not found")
else()
	message(STATUS "Found QwtPolar include files - ${QWTPOLAR_INCLUDE_DIRS}/qwt_polar_plot.h")
endif()

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)
message(CMAKE_PATH ${CMAKE_PREFIX_PATH})
include_directories(
	${Boost_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Concurrent_INCLUDE_DIRS}
	${Qt5Qml_INCLUDE_DIRS}
	${Qt5UiTools_INCLUDE_DIRS}
	${QWT_INCLUDE_DIRS}
	${QWTPOLAR_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${Qt5Xml_INCLUDE_DIRS}
	${SCOPY_INCLUDE_DIRS}
	${LIBSIGROK_DECODE_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${GLIBCONFIG_INCLUDE_DIRS}
	${GLIBMM_INCLUDE_DIRS}
	${GLIBMMCONFIG_INCLUDE_DIRS}
	${SIGCPP_INCLUDE_DIRS}
	${SIGCPPCONFIG_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
	${BREAKPAD_INCLUDE_DIRS}
	${BREAKPAD_INCLUDE_DIRS}/breakpad
)

link_directories(
	${Boost_LIBRARY_DIRS}
	${GNURADIO_RUNTIME_LIBRARY_DIRS}
	${BREAKPAD_LIBRARIES}
	)

FILE(GLOB SRC_LIST  src/*.cpp src/*.cc
	src/patterngenerator/*.cpp
	src/patterngenerator/*/*.cpp
	src/logicanalyzer/*.cpp
	src/logicanalyzer/*/*.cpp
)

FILE(GLOB M2KSCOPE_UIS ui/patterns/*.ui ui/*.ui)
qt5_wrap_ui (m2kscope_FORMS_HEADERS ${M2KSCOPE_UIS})

FILE(GLOB M2KSCOPE_RESOURCES resources/resources.qrc)
qt5_add_resources(m2kscope_RESOURCES ${M2KSCOPE_RESOURCES})

if (WIN32)
        set(RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources/properties.rc)
        configure_file(properties.rc.cmakein ${RESOURCES} @ONLY)
endif()

add_definitions(-DQT_NO_KEYWORDS)

if (APPLE)
	option(ENABLE_APPLICATION_BUNDLE "Enable application bundle for OSX" ON)
endif(APPLE)

if (ENABLE_APPLICATION_BUNDLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

	set(PKGINFO ${CMAKE_BINARY_DIR}/PkgInfo)
	file(WRITE ${PKGINFO} "APPLScopy")
	set_source_files_properties(${PKGINFO} PROPERTIES MACOSX_PACKAGE_LOCATION .)

	set(QT_CONF ${CMAKE_BINARY_DIR}/qt.conf)
	file(WRITE ${QT_CONF} "")
	set_source_files_properties(${QT_CONF} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	set(ICON_FILE ${CMAKE_SOURCE_DIR}/resources/Scopy.icns)
	set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	set(CMAKE_EXE_LINKER_FLAGS "-Wl,-headerpad_max_install_names -Wl,-search_paths_first ${CMAKE_EXE_LINKER_FLAGS}")

	foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS})
		get_target_property(_loc ${plugin} LOCATION)
		get_filename_component(_name ${_loc} NAME)
		get_filename_component(_dir ${_loc} DIRECTORY)
		get_filename_component(_dir ${_dir} NAME)

		set_source_files_properties(${_loc} PROPERTIES MACOSX_PACKAGE_LOCATION plugins/${_dir})
		set(QT_PLUGINS ${QT_PLUGINS} ${_loc})
		set(BUNDLED_QT_PLUGINS ${BUNDLED_QT_PLUGINS} ${CMAKE_BINARY_DIR}/Scopy.app/Contents/plugins/${_dir}/${_name})
	endforeach()

	file(GLOB_RECURSE DECODERS ${LIBSIGROK_DECODERS_DIR}/*.py)
	foreach(_decoder ${DECODERS})
		file(RELATIVE_PATH _file ${LIBSIGROK_DECODERS_DIR} ${_decoder})
		get_filename_component(_path ${_file} DIRECTORY)
		set_source_files_properties(${_decoder} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS/decoders/${_path})
	endforeach()

	install(CODE "
		set(BU_CHMOD_BUNDLE_ITEMS ON)
		include(BundleUtilities)
		fixup_bundle(\"${CMAKE_BINARY_DIR}/Scopy.app\" \"${BUNDLED_QT_PLUGINS}\" \"${CMAKE_SOURCE_DIR}\")"
	)

	set(OSX_BUNDLE MACOSX_BUNDLE)
	set(EXTRA_BUNDLE_FILES ${QT_PLUGINS} ${ICON_FILE} ${PKGINFO} ${QT_CONF} ${DECODERS})
endif()

#dump_cmake_variables("iio")

string(TIMESTAMP TODAY "%Y-%m-%d")
string(TIMESTAMP NOW "%H:%M:%S")
cmake_host_system_information(RESULT BUILD_HOST QUERY HOSTNAME)

set(APPVEYOR_INFO)
if($ENV{APPVEYOR})
	set(APPVEYOR_INFO ${APPVEYOR_INFO}Built\ on\ Appveyor\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}url:\ $ENV{APPVEYOR_URL}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}api_url:\ $ENV{APPVEYOR_API_URL}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}acc_name:\ $ENV{APPVEYOR_ACCOUNT_NAME}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}prj_name:\ $ENV{APPVEYOR_PROJECT_NAME}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}build_id:\ $ENV{APPVEYOR_BUILD_ID}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}build_nr:\ $ENV{APPVEYOR_BUILD_NUMBER}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}build_version:\ $ENV{APPVEYOR_BUILD_VERSION}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}job_id:\ $ENV{APPVEYOR_JOB_ID}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}job_name:\ $ENV{APPVEYOR_JOB_NAME}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}job_nr:\ $ENV{APPVEYOR_JOB_NUMBER}\n)
	set(APPVEYOR_INFO ${APPVEYOR_INFO}job_link:\ $ENV{APPVEYOR_URL}/project/$ENV{APPVEYOR_ACCOUNT_NAME}/$ENV{APPVEYOR_PROJECT_NAME}/builds/$ENV{APPVEYOR_BUILD_ID}/job/$ENV{APPVEYOR_JOB_ID}\n)		
	if(EXISTS ${CMAKE_SOURCE_DIR}/scopy-mingw-build-status)
		message(scopy-mingw-build-status found .. populating)
		FILE(READ ${CMAKE_SOURCE_DIR}/scopy-mingw-build-status SCOPY_MINGW_BUILD_STATUS_INFO)
	endif()
else()
	set(APPVEYOR_INFO ${APPVEYOR_INFO}Built\ locally\n)
endif()

# TODO: Pack these in a GLOB and run foreach
configure_file(resources/buildinfo.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.html)
configure_file(resources/scopy_osp.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy_osp.html)
configure_file(resources/credits.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/credits.html)
configure_file(resources/about.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/about.html)

SET(ABOUT_HTML_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources/buildinfo.html ${CMAKE_CURRENT_BINARY_DIR}/resources/scopy_osp.html ${CMAKE_CURRENT_BINARY_DIR}/resources/credits.html ${CMAKE_CURRENT_BINARY_DIR}/resources/about.html)

set(ABOUT_HTML_QRC_SOURCES)
foreach(file ${ABOUT_HTML_SOURCES})
	get_filename_component(file_name ${file} NAME)
	set(ABOUT_HTML_QRC_SOURCES "${ABOUT_HTML_QRC_SOURCES}\n<file>${file_name}</file>")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/resources/aboutpage.qrc
	${CMAKE_BINARY_DIR}/aboutpage.qrc
	@ONLY)

qt5_add_resources(ABOUT_PAGE_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/aboutpage.qrc)

option(CLONE_IIO_EMU "Clone iio-emu" ON)
if (CLONE_IIO_EMU)
	message(STATUS "Checking for iio-emu")
	execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE GIT_SUBMOD_RESULT)
	if(NOT GIT_SUBMOD_RESULT EQUAL "0")
		message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
	endif()
	add_subdirectory(iio-emu)
endif()

add_executable(${PROJECT_NAME} WIN32 ${OSX_BUNDLE}
		${SRC_LIST}
		${RESOURCES}
		${m2kscope_RESOURCES}
		${m2kscope_FORMS_HEADERS}
		${EXTRA_BUNDLE_FILES}
		${TRANSLATION_RESOURCES}
		${ABOUT_PAGE_RESOURCES}
)

if (WIN32 OR ENABLE_APPLICATION_BUNDLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME Scopy)
endif()

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
		${BREAKPAD_LIBRARIES}
		${BREAKPADCLIENT_LIBRARIES}
		${Qt5Widgets_LIBRARIES}
		${Qt5Concurrent_LIBRARIES}
		${Qt5Qml_LIBRARIES}
		${Qt5UiTools_LIBRARIES}
		gnuradio::gnuradio-runtime
		gnuradio::gnuradio-analog
		gnuradio::gnuradio-blocks
		gnuradio::gnuradio-fft
		gnuradio::gnuradio-filter
		gnuradio::gnuradio-pmt
		gnuradio::gnuradio-iio
		gnuradio::gnuradio-scopy
		gnuradio::gnuradio-m2k
		${Boost_LIBRARIES}
		${QWT_LIBRARIES}
		${QWTPOLAR_LIBRARIES}
		${Qt5Svg_LIBRARIES}
		${Qt5Xml_LIBRARIES}
		${MATIO_LIBRARIES}
		${LIBSIGROK_DECODE_LIBRARIES}
		${GLIBMM_LIBRARIES}
		${SIGCPP_LIBRARIES}
		${GLIB_LIBRARIES}
		libm2k::libm2k
)

if (NOT WIN32)
	find_library(PTHREAD_LIBRARIES pthread)
	if (PTHREAD_LIBRARIES)
		target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${PTHREAD_LIBRARIES})
	endif()
endif()

configure_file(Info.plist.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Info.plist @ONLY)

option(WITH_NATIVEDIALOGS "Enable native file dialogs for Scopy" ON)

if (NOT ${WITH_NATIVEDIALOGS})
	add_definitions(-DNONATIVE)
endif()

# Compiler options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

#List of warnings to be treated as errors
target_compile_options(${PROJECT_NAME} PUBLIC
	-Werror=return-type
	-Werror=uninitialized
	-Werror=init-self
	-Werror=switch
)

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
)



set(CMAKE_INSTALL_DOCDIR "${CMAKE_CURRENT_BINARY_DIR}/doc")
find_package(Doxygen)
if(DOXYGEN_FOUND)
	option(WITH_DOC "Generate documentation with Doxygen" OFF)
	if (WITH_DOC)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile_API.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
		set(HTML_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
		#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc DESTINATION ${HTML_DEST_DIR})

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
			)

		if(NOT SKIP_INSTALL_ALL)
			install(DIRECTORY ${HTML_DEST_DIR} DESTINATION ${CMAKE_INSTALL_DOCDIR})
		endif()
	endif()
else()
	message(STATUS "Doxygen not found, API documentation won't be generated")
endif()


configure_file(scopy.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.iss @ONLY)
configure_file(scopy-32.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy-32.iss @ONLY)
configure_file(scopy-64.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy-64.iss @ONLY)
configure_file(config.h.cmakein ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if (NOT ENABLE_APPLICATION_BUNDLE)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/scopy.desktop.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop DESTINATION share/applications)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon_small.svg DESTINATION share/icons/hicolor/apps/scalable RENAME scopy.svg)
endif()

configure_file(qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf @ONLY)
