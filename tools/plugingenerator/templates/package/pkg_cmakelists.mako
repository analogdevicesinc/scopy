cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE ${id})

message(STATUS "building package: " ${"${SCOPY_MODULE}"})

project(scopy-package-${"${SCOPY_MODULE}"} VERSION 0.1 LANGUAGES CXX)

configure_file(
	manifest.json.cmakein
	${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"}/MANIFEST.json @ONLY
)
%if en_style:
message("Generate pkg style")
include(ScopyStyle)
generate_style("--core" ${"${CMAKE_CURRENT_SOURCE_DIR}"}/style  ${"${CMAKE_SOURCE_DIR}"}/gui/include/gui/pkg-style ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"})
%endif
%if en_translation:
message("Generate pkg translations")
include(ScopyTranslation)
if(ENABLE_TRANSLATION)
	generate_translations(${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"}/translations)
	qt_add_resources(SCOPY_RESOURCES ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"}/translations.qrc)
endif()
%endif
message("Including plugins")
file(GLOB CHILDREN RELATIVE ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins/*)

if(EXISTS ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins)
    foreach(child ${"${CHILDREN}"})
        if(IS_DIRECTORY ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins/${"${child}"})
            if(${"${CMAKE_SYSTEM_NAME}"} MATCHES "Windows")
                set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"}/plugins)
            elseif(${"${CMAKE_SYSTEM_NAME}"} MATCHES "Darwin")
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${"${CMAKE_BINARY_DIR}"}/Scopy.app/Contents/MacOS/${"${PACKAGE_NAME}"}/plugins")
            else()
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${PACKAGE_NAME}"}/plugins)
            endif()
            add_subdirectory(plugins/${"${child}"})
        endif()
    endforeach()
endif()