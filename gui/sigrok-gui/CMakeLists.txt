cmake_minimum_required(VERSION 3.9)
set(SCOPY_MODULE sigrok-gui)
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

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/*.cc
	src/binding/*.cpp
	src/prop/*.cpp
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/binding/*.hpp
	include/${SCOPY_MODULE}/prop/*.hpp
)

file(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

configure_file(
	include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h @ONLY
)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

message(STATUS "Building " ${PROJECT_NAME})
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(
	${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LIBSIGROK_DECODE_INCLUDE_DIRS}
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/
		${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/binding
		${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/prop
		${LIBSIGROK_DECODE_INCLUDE_DIRS}
		scopy-gui
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})

target_link_libraries(
	${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Widgets scopy-gui ${LIBSIGROK_DECODE_LINK_LIBRARIES}
)

set(SCOPY_SIGROK_GUI ON PARENT_SCOPE)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_PATH} RUNTIME DESTINATION ${SCOPY_DLL_PATH})
