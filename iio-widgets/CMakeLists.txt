cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE iio-widgets)
project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/*.cc
	src/datastrategy/*.cc
	src/datastrategy/*.cpp
	src/guistrategy/*.cc
	src/guistrategy/*.cpp
	src/savestrategy/*.cc
	src/savestrategy/*.cpp
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/datastrategy/*.h
	include/${SCOPY_MODULE}/datastrategy/*.hpp
	include/${SCOPY_MODULE}/guistrategy/*.h
	include/${SCOPY_MODULE}/guistrategy/*.hpp
	include/${SCOPY_MODULE}/savestrategy/*.h
	include/${SCOPY_MODULE}/savestrategy/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(SCOPY_QT_COMPONENTS Core Widgets)

find_library(IIO_LIBRARIES NAMES iio libiio REQUIRED)
find_path(IIO_INCLUDE_DIRS iio.h REQUIRED)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(
	${PROJECT_NAME} INTERFACE ${IIO_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include
				  ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
)
target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/datastrategy
		${CMAKE_CURRENT_SOURCE_DIR}/src/guistrategy
		${CMAKE_CURRENT_SOURCE_DIR}/src/savestrategy
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/datastrategy
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/guistrategy
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/savestrategy
		${IIO_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME} PUBLIC scopy-common scopy-gui scopy-core scopy-iioutil)

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(${PROJECT_NAME} PUBLIC ${SCOPY_QT_LIBRARIES} ${IIO_LIBRARIES} scopy-gui scopy-iioutil)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_PATH} RUNTIME DESTINATION ${SCOPY_DLL_PATH})
