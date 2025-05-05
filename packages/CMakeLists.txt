cmake_minimum_required(VERSION 3.9)

project(scopy-packages VERSION 0.1 LANGUAGES CXX)

# If DEV_MODE, remove all previously built packages
if(SCOPY_DEV_MODE)
	file(REMOVE_RECURSE ${SCOPY_PACKAGE_BUILD_PATH})
endif()

# Include utils
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(PackageUtils)

add_packages(${CMAKE_CURRENT_SOURCE_DIR} ${SCOPY_PACKAGE_BUILD_PATH})
