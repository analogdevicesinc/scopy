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

configure_file(config.h.cmakein ${CMAKE_CURRENT_SOURCE_DIR}/core/include/scopy/core/config.hpp @ONLY)
