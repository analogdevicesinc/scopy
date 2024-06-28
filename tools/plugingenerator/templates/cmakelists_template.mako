% if "cmake_min_required" in config:
cmake_minimum_required(VERSION ${config['cmake_min_required']})
% else:
cmake_minimum_required(VERSION 3.9)
%endif

set(SCOPY_MODULE ${scopy_module})

message(STATUS "building plugin: " ${"${SCOPY_MODULE}"})

project(scopy-${"${SCOPY_MODULE}"} VERSION 0.1 LANGUAGES CXX) 

set(PLUGIN_DISPLAY_NAME ${plugin_display_name.upper()})
set(PLUGIN_DESCRIPTION ${plugin_description.upper()})

include(GenerateExportHeader) 

# TODO: split stylesheet/resources and add here TODO: export header files correctly

% if "cxx_standard" in config:
set(CMAKE_CXX_STANDARD ${config['cxx_standard']})
% else:
set(CMAKE_CXX_STANDARD 17)
% endif
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

set(CMAKE_AUTOUIC_SEARCH_PATHS ${"${CMAKE_CURRENT_SOURCE_DIR}"}/ui) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 

set(CMAKE_INCLUDE_CURRENT_DIR ON) 

set(CMAKE_CXX_VISIBILITY_PRESET hidden) 
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE) 

file(GLOB SRC_LIST src/*.cpp src/*.cc) 
file(GLOB HEADER_LIST include/${"${SCOPY_MODULE}"}/*.h include/${"${SCOPY_MODULE}"}/*.hpp) 
file(GLOB UI_LIST ui/*.ui) 

% if "enable_testing" in config:
set(ENABLE_TESTING ${config['enable_testing']})
% else: 
set(ENABLE_TESTING "OFF")
% endif
if(ENABLE_TESTING) 
    add_subdirectory(test) 
endif() 

set(PROJECT_SOURCES ${"${SRC_LIST}"} ${"${HEADER_LIST}"} ${"${UI_LIST}"}) 
find_package(Qt${"${QT_VERSION_MAJOR}"} COMPONENTS REQUIRED Widgets Core) 

qt_add_resources(PROJECT_RESOURCES res/resources.qrc) 
add_library(${"${PROJECT_NAME}"} SHARED ${"${PROJECT_SOURCES}"} ${"${PROJECT_RESOURCES}"}) 

generate_export_header( 
    ${"${PROJECT_NAME}"} EXPORT_FILE_NAME ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/${"${SCOPY_MODULE}"}/${"${PROJECT_NAME}"}_export.h 
)

configure_file(
	include/${"${SCOPY_MODULE}"}/scopy-${"${SCOPY_MODULE}"}_config.h.cmakein
	${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/${"${SCOPY_MODULE}"}/scopy-${"${SCOPY_MODULE}"}_config.h @ONLY
)

target_include_directories(${"${PROJECT_NAME}"} INTERFACE ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include) 
target_include_directories(${"${PROJECT_NAME}"} PRIVATE ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/${"${SCOPY_MODULE}"}) 

target_include_directories(${"${PROJECT_NAME}"} PUBLIC scopy-pluginbase scopy-gui) 

target_link_libraries( 
    ${"${PROJECT_NAME}"} 
    PUBLIC Qt::Widgets 
        Qt::Core 
        scopy-pluginbase 
        scopy-gui 
        scopy-iioutil 
)

if(${"${CMAKE_SYSTEM_NAME}"} MATCHES "Windows")
	configureinstallersettings(${"${SCOPY_MODULE}"} ${"${PLUGIN_DESCRIPTION}"} FALSE)
endif()

set(${scopy_module}_TARGET_NAME ${"${PROJECT_NAME}"} PARENT_SCOPE)