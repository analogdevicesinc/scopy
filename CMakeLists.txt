cmake_minimum_required(VERSION 3.9.6)

project(Scopy LANGUAGES CXX)
include(cmake/standardProjectSettings.cmake)
include(cmake/preventInSourceBuilds.cmake)

# allow static analysis
include(cmake/staticAnalyzers.cmake)

# link this library to set project options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_11)

# link this library to set project warnings
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall)

# enable sanitizers for project options
include(cmake/sanitizers.cmake)
enable_sanitizers(project_options)

# allow code style analysis
include(cmake/codeStyle.cmake)

# scopy core library
add_subdirectory(core)

enable_testing()

# scopy gui library
add_subdirectory(gui)

# scopy plugins
add_subdirectory(plugins)

# scopy application
add_subdirectory(app)

include(cmake/doxygen.cmake)
enable_doxygen("scopy::gui;")

set(PYTHON_VERSION python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})

# Get the GIT hash of the latest commit
include(FindGit OPTIONAL)
if(GIT_FOUND)
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

if(NOT SCOPY_VERSION_GIT)
    set(SCOPY_VERSION_GIT v${PROJECT_VERSION})
endif()

option(BREAKPAD_HANDLER "Build with breakpad exception handler " OFF)
message(STATUS BREAKPAD_HANDLER - ${BREAKPAD_HANDLER})
if(${BREAKPAD_HANDLER})
    message("-- Building with breakpad crash handler")
    set(BREAKPAD_HANDLER_BOOL 1)
    find_library(BREAKPAD_LIBRARIES NAMES breakpad)
    find_library(BREAKPADCLIENT_LIBRARIES NAMES breakpad_client)
    find_path(BREAKPAD_INCLUDE_DIRS breakpad)
else()
    message("-- Building without breakpad crash handler")
    set(BREAKPAD_HANDLER_BOOL 0)
    set(BREAKPAD_LIBRARIES "")
    set(BREAKPAD_INCLUDE_DIRS "")
    set(BREAKPADCLIENT_LIBRARIES "")

endif()

FILE(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/resources/translations/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5 COMPONENTS LinguistTools REQUIRED)

# Generate .qm files from the .ts files
qt5_add_translation(QM_FILES ${TS_FILES})

set(TRANSLATIONS)
foreach(file ${TS_FILES})
	get_filename_component(file_name ${file} NAME_WE)
	set(TRANSLATIONS "${TRANSLATIONS}\n<file>${file_name}.qm</file>")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/gui/resources/translations.qrc
	${CMAKE_BINARY_DIR}/gui/translations.qrc
	@ONLY)

qt5_add_resources(TRANSLATION_RESOURCES ${CMAKE_BINARY_DIR}/translations.qrc)

configure_file(
    config.h.cmakein
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include/scopy/core/config.hpp @ONLY
)

if (WIN32)
	set(RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources/properties.rc)
	configure_file(properties.rc.cmakein ${RESOURCES} @ONLY)
endif()

# TODO: Pack these in a GLOB and run foreach
configure_file(gui/resources/html-pages/buildinfo.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.html)
configure_file(gui/resources/html-pages/scopy_osp.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy_osp.html)
configure_file(gui/resources/html-pages/credits.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/credits.html)
configure_file(gui/resources/html-pages/about.html.cmakein ${CMAKE_CURRENT_BINARY_DIR}/about.html)

SET(ABOUT_HTML_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources/html-pages/buildinfo.html ${CMAKE_CURRENT_BINARY_DIR}/resources/scopy_osp.html ${CMAKE_CURRENT_BINARY_DIR}/resources/credits.html ${CMAKE_CURRENT_BINARY_DIR}/resources/about.html)

set(ABOUT_HTML_QRC_SOURCES)
foreach(file ${ABOUT_HTML_SOURCES})
	get_filename_component(file_name ${file} NAME)
	set(ABOUT_HTML_QRC_SOURCES "${ABOUT_HTML_QRC_SOURCES}\n<file>${file_name}</file>")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/gui/resources/aboutpage.qrc
	${CMAKE_BINARY_DIR}/aboutpage.qrc
	@ONLY)

qt5_add_resources(ABOUT_PAGE_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/aboutpage.qrc)
