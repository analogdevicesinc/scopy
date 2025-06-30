cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE test-plugins)
set(CURRENT_PKG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PACKAGE_NAME ${SCOPY_MODULE})

message(STATUS "building package: " ${SCOPY_MODULE})

project(scopy-package-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

configure_file(
	manifest.json.cmakein
	${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/MANIFEST.json @ONLY
)
message("Including plugins")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
	add_plugins(${CMAKE_CURRENT_SOURCE_DIR}/plugins ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins)
	install_plugins(${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins ${SCOPY_PACKAGE_INSTALL_PATH}/${SCOPY_MODULE}/plugins "scopy")
endif()
install_pkg(${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE} ${SCOPY_PACKAGE_INSTALL_PATH}/${SCOPY_MODULE})
