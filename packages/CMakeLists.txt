cmake_minimum_required(VERSION 3.9)

project(scopy-packages VERSION 0.1 LANGUAGES CXX)

# Include utils
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(PackageUtils)

# Set packages location
set(PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
add_all_packages(${PACKAGES_DIR})
