cmake_minimum_required(VERSION 3.9)
set(SCOPY_MODULE gui)
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

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/widgets/*.cpp
	src/*.cc
	src/binding/*.cpp
	src/prop/*.cpp
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/widgets/*.h
	include/${SCOPY_MODULE}/binding/*.hpp
	include/${SCOPY_MODULE}/prop/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

qt_add_resources(PROJECT_RESOURCES res/resources.qrc)

if(ANDROID)
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt_${ANDROID_ABI})
else()
	set(QWT_QT_VERSION qt5)
	find_path(
		QWT_INCLUDE_DIRS
		NAMES qwt_global.h
		HINTS ${CMAKE_INSTALL_PREFIX}/include
		      /include
		      /usr/local/include
		      /usr/include
		      /opt/local/include
		      /sw/include/usr/local/lib/qwt.framework/Headers
		PATH_SUFFIXES
			qwt-${QWT_QT_VERSION}
			qwt
			qwt6
			qwt5
			${QWT_QT_VERSION}/qwt
			REQUIRED
	)

	find_library(
		QWT_LIBRARIES
		NAMES qwt
		HINTS ${CMAKE_INSTALL_PREFIX}/lib
		      /usr/lib
		      /usr/lib64
		      /usr/local/lib
		      /usr/local/lib64
		      /opt/local/lib
		      /opt/local/lib64
		      REQUIRED
	)
	message("QWT LIBRARIES: " ${QWT_LIBRARIES})

endif()

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Xml Svg REQUIRED)

add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost COMPONENTS system filesystem thread chrono REQUIRED)

message(STATUS "Building " ${PROJECT_NAME})
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)
configure_file(
	include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h @ONLY
)

target_include_directories(
	${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
				  ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/widgets
)
target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/widgets scopy-common scopy-pluginbase
)

target_include_directories(
	${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${QWT_INCLUDE_DIRS}
			       ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt${QT_VERSION_MAJOR}::Widgets
	       Qt${QT_VERSION_MAJOR}::Xml
	       Qt${QT_VERSION_MAJOR}::Svg
	       ${QWT_LIBRARIES}
	       ${Boost_LIBRARIES}
	       scopy-common
	       scopy-pluginbase
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})
install(DIRECTORY res/ DESTINATION resources/ COMPONENT ${SCOPY_PDK})
