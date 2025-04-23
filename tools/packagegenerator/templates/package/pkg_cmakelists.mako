cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE ${id})
set(CURRENT_PKG_PATH ${"${CMAKE_CURRENT_SOURCE_DIR}"})
set(PACKAGE_NAME ${"${SCOPY_MODULE}"})

message(STATUS "building package: " ${"${SCOPY_MODULE}"})

project(scopy-package-${"${SCOPY_MODULE}"} VERSION 0.1 LANGUAGES CXX)

configure_file(
	manifest.json.cmakein
	${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/MANIFEST.json @ONLY
)
%if en_style:
message("Generate pkg style")
include(ScopyStyle)
generate_style("--core" ${"${CMAKE_CURRENT_SOURCE_DIR}"}/style  ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/${"${SCOPY_MODULE}"} ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"})
%endif
%if en_translation:
message("Generate pkg translations")
include(ScopyTranslation)
if(ENABLE_TRANSLATION)
	generate_translations(${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/translations)
	qt_add_resources(SCOPY_RESOURCES ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/translations.qrc)
endif()
%endif
message("Including plugins")
if(EXISTS ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins)
	add_plugins(${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/plugins)
endif()