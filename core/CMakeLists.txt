cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE core)
project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(SCOPY_QT_COMPONENTS Core Widgets Concurrent Network)

file(GLOB SRC_LIST src/*.cpp src/*.cc)
file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
file(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

option(WITH_PYTHON "Enable Python" ON)
if(${WITH_PYTHON})
	set(Python_ADDITIONAL_VERSIONS 3)
	if(PYTHON_EXECUTABLE)
		message(STATUS "Using custom Python EXECUTABLE: ${PYTHON_EXECUTABLE}")
		set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
	else()
		message(STATUS "Using default Python EXECUTABLE")
	endif()

	if(CMAKE_SYSTEM_PROCESSOR MATCHES arm)
		find_package(Python3 3.9 EXACT REQUIRED COMPONENTS Interpreter)
		message (STATUS "Host Python Interpreter " ${Python3_EXECUTABLE})
		set(Python3_EXECUTABLE ${CMAKE_SYSROOT}/bin/python3.9)
		message (STATUS "Target Python Interpreter " ${Python3_EXECUTABLE})
	else()
		find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
		message (STATUS "Python Interpreter " ${Python3_EXECUTABLE})
		message (STATUS "Python Libraries " ${Python3_LIBRARIES})
	endif()
	set(PYTHON_VERSION python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR} CACHE STRING "PYTHON_USED")
	set(PYTHON_VERSION ${PYTHON_VERSION} PARENT_SCOPE)
	if(NOT Python3_FOUND)
		set(WITH_PYTHON OFF)
		message(STATUS "Python not found")
	endif()
	set(BUILD_PYTHON_LIBRARY_DIRS ${Python3_LIBRARY_DIRS}/${PYTHON_VERSION})
endif()

option(WITH_SIGROK "Search for libsigrokdecode" ON)
if(WITH_SIGROK)
	find_package(PkgConfig)
	set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
	pkg_check_modules(GLIB REQUIRED glib-2.0)
	pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
	pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)
	if(NOT LIBSIGROK_DECODE_FOUND)
		set(WITH_SIGROK OFF)
		message(STATUS "Libsigrokdecode not found")
	endif()
endif()

option(SCOPY_DEV_MODE "Enable development specific scopy behavior - autoconnect to a context on start" FALSE)

configure_file(
	include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h @ONLY
)
set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
		${GLIB_INCLUDE_DIRS}
		${GLIBCONFIG_INCLUDE_DIRS}
		${LIBSIGROK_DECODE_INCLUDE_DIRS}
		${Python3_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-pluginbase scopy-iioutil scopy-common)

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC ${SCOPY_QT_LIBRARIES}
	       scopy-common
	       scopy-gui
	       scopy-pluginbase
	       scopy-iioutil
	       ${LIBSIGROK_DECODE_LINK_LIBRARIES}
	       ${Python3_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_PATH} RUNTIME DESTINATION ${SCOPY_DLL_PATH})
