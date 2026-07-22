# iio-backend-v0 — libiio v0 (legacy) plugin. dlopen'd at runtime by IIOBackendLoader,
# which looks for "libiio-backend-v0.so". Built only when libiio.so.0 is present.
if(NOT (IIO_V0_LIB AND IIO_V0_INCLUDE))
    message(STATUS "libiio v0 not found — skipping iio-backend-v0")
    return()
endif()

file(GLOB V0_HEADERS include/v0/*.h)
file(GLOB V0_SOURCES src/*.cpp)

add_library(iio-backend-v0 SHARED
    ${V0_HEADERS}
    ${V0_SOURCES}
)

# Produce exactly libiio-backend-v0.so so the loader's
# dlopen("libiio-backend-v0.so") resolves it.
set_target_properties(iio-backend-v0 PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "iio-backend-v0"
)

target_include_directories(iio-backend-v0
    PRIVATE include include/v0 ${IIO_V0_INCLUDE}
)

target_link_libraries(iio-backend-v0
    PRIVATE Qt6::Core Qt6::Concurrent ${IIO_V0_LIB} iio-utils
)
