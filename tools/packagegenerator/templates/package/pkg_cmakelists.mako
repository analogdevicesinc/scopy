cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE ${id})
set(CURRENT_PKG_PATH ${"${CMAKE_CURRENT_SOURCE_DIR}"})
set(PACKAGE_NAME ${"${SCOPY_MODULE}"})

set(PACKAGE_DISPLAY_NAME "${title}")
set(PACKAGE_DESCRIPTION "${description}")

project(scopy-package-${"${SCOPY_MODULE}"} VERSION 0.1 LANGUAGES CXX)

configure_file(
	manifest.json.cmakein
	${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/MANIFEST.json @ONLY
)
%if en_style:
message(STATUS "Generate pkg style")
include(ScopyStyle)
generate_style("--core" ${"${CMAKE_CURRENT_SOURCE_DIR}"}/style  ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/${"${SCOPY_MODULE}"} ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"})
%endif
%if en_translation:
message(STATUS "Generate pkg translations")
include(ScopyTranslation)
if(ENABLE_TRANSLATION)
	generate_translations(${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/translations)
	qt_add_resources(SCOPY_RESOURCES ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/translations.qrc)
endif()
%endif
%if emu_xml:
include_emu_xml(${"${CMAKE_CURRENT_SOURCE_DIR}"}/emu-xml ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/emu-xml)
%endif
include_resources(${"${CMAKE_CURRENT_SOURCE_DIR}"}/resources ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/resources)
message(STATUS "Including plugins for ${"${SCOPY_MODULE}"}")
if(EXISTS ${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins)
	add_plugins(${"${CMAKE_CURRENT_SOURCE_DIR}"}/plugins ${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/plugins)
	install_plugins(
		${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"}/plugins
		${"${SCOPY_PACKAGE_INSTALL_PATH}"}/${"${SCOPY_MODULE}"}/plugins "scopy"
	)
endif()
install_pkg(${"${SCOPY_PACKAGE_BUILD_PATH}"}/${"${SCOPY_MODULE}"} ${"${SCOPY_PACKAGE_INSTALL_PATH}"}/${"${SCOPY_MODULE}"})

if(${"${CMAKE_SYSTEM_NAME}"} MATCHES "Windows")
	set(INSTALLER_DESCRIPTION "${"${PACKAGE_DISPLAY_NAME}"} - ${"${PACKAGE_DESCRIPTION}"}")
	configureinstallersettings(${"${SCOPY_MODULE}"} ${"${INSTALLER_DESCRIPTION}"} FALSE)
endif()
