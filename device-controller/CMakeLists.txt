cmake_minimum_required(VERSION 3.21)
project(device-controller VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Test)
find_package(QCoro6 REQUIRED COMPONENTS Core Coro)

# libiio: reuse the exact library/headers the parent Scopy build already resolved
# (IIO_LIBRARIES / IIO_INCLUDE_DIRS, set in the root CMakeLists before this
# add_subdirectory). This keeps the v0 backend linking the SAME libiio as the rest of
# Scopy — no accidental version divergence — and stays portable across every platform
# Scopy targets (Linux/macOS/Windows/ARM), unlike a hardcoded libiio.so.0 + x86_64
# multiarch search. Scopy pins libiio v0.26, so "Scopy's libiio" is v0.
# Fall back to a generic search only if a parent didn't provide these (defensive; the
# vendored device-controller is only ever built embedded).
if(NOT IIO_LIBRARIES OR NOT IIO_INCLUDE_DIRS)
    find_library(IIO_LIBRARIES NAMES iio libiio REQUIRED)
    find_path(IIO_INCLUDE_DIRS NAMES iio.h REQUIRED)
endif()
message(STATUS "device-controller libiio: ${IIO_LIBRARIES} (include: ${IIO_INCLUDE_DIRS})")

# libiio version wiring for this integration. Only v0 is supported and it is linked
# DIRECTLY into `component` (no dlopen); the dlopen IIOBackendLoader stays in the tree
# but dormant. The option is reserved for future v0/v1 selection — any value other than
# 0 is rejected on purpose so a premature v1/runtime-selection build fails loudly.
set(SCOPY_LIBIIO_VERSION "0" CACHE STRING "libiio backend version to link directly (only 0 supported)")
if(NOT SCOPY_LIBIIO_VERSION STREQUAL "0")
    message(FATAL_ERROR
        "SCOPY_LIBIIO_VERSION='${SCOPY_LIBIIO_VERSION}' is not supported yet. "
        "Only 0 (libiio v0, direct-linked) is wired for this integration.")
endif()

# libiio is a hard requirement in direct-link mode (no runtime fallback this stage).
if(NOT (IIO_LIBRARIES AND IIO_INCLUDE_DIRS))
    message(FATAL_ERROR
        "libiio (v0) is required for the direct-linked device-controller backend "
        "but was not found.")
endif()

add_subdirectory(core)
add_subdirectory(iio-utils)
add_subdirectory(iio-backend-v0)
# iio-backend-v1 is intentionally NOT built for this integration (v0-only). The source
# is retained in the tree for future libiio v0/v1 support via the dormant loader.
# add_subdirectory(iio-backend-v1)
add_subdirectory(component)
