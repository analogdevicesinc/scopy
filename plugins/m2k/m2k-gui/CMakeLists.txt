cmake_minimum_required(VERSION 3.9)
set(SCOPY_MODULE m2k-gui)
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

file(GLOB SRC_LIST src/*.cpp src/*.cc)
file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
file(GLOB UI_LIST ui/*.ui)
if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

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

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Xml Svg REQUIRED)
option(SCOPY_ENABLE_GR_GUI "Build gnuradio addons for scopy-gui" ON)
option(SCOPY_ENABLE_SIGROK_GUI "Build sigrok addons for scopy-gui" ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCOPY_DLL_BUILD_PATH})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/Frameworks")
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_DLL_BUILD_PATH})
endif()

if(SCOPY_ENABLE_GR_GUI)
	add_subdirectory(gr-gui)
	set(SCOPY_HAVE_GR_GUI ON)
endif()
if(SCOPY_ENABLE_SIGROK_GUI)
	add_subdirectory(sigrok-gui)
	set(SCOPY_HAVE_SIGROK_GUI ON)
endif()

add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost COMPONENTS system filesystem thread chrono REQUIRED)

message(STATUS "Building " ${PROJECT_NAME})
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)
configure_file(
	include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_config.h @ONLY
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/
		scopy-common
		scopy-pluginbase
		scopy-gui
		scopy-sigrok-gui
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
	       scopy-gui
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
