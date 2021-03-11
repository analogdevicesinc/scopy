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
