cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE apollo-ad9084)
set(CURRENT_PKG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PACKAGE_NAME ${SCOPY_MODULE})

message(STATUS "building package: " ${SCOPY_MODULE})

project(scopy-package-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

configure_file(
	manifest.json.cmakein
	${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/MANIFEST.json @ONLY
)
include_emu_xml(${CMAKE_CURRENT_SOURCE_DIR}/emu-xml ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/emu-xml)
message("Including plugins")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
	add_plugins(${CMAKE_CURRENT_SOURCE_DIR}/plugins ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins)
endif()