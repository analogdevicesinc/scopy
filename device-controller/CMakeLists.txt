cmake_minimum_required(VERSION 3.21)
project(device-controller VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Test)
find_package(QCoro6 REQUIRED COMPONENTS Core Coro)

# libiio v0 backend plugin
find_library(IIO_V0_LIB
    NAMES libiio.so.0
    PATHS /usr/local/lib /usr/lib/x86_64-linux-gnu
)
find_path(IIO_V0_INCLUDE
    NAMES iio.h
)

# libiio v1 backend plugin
find_library(IIO_V1_LIB
    NAMES libiio.so.1
    PATHS /usr/local/lib /usr/lib/x86_64-linux-gnu
)
find_path(IIO_V1_INCLUDE
    NAMES iio/iio.h
)

message(STATUS "IIO_V0_LIB: ${IIO_V0_LIB}")
message(STATUS "IIO_V1_LIB: ${IIO_V1_LIB}")

enable_testing()

add_subdirectory(core)
add_subdirectory(iio-utils)
add_subdirectory(iio-backend-v0)
add_subdirectory(iio-backend-v1)
add_subdirectory(component)
