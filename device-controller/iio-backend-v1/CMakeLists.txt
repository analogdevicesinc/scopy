# iio-backend-v1 — libiio v1 (new API) plugin. dlopen'd at runtime by IIOBackendLoader,
# which looks for "libiio-backend-v1.so". Built only when libiio.so.1 is present.
if(NOT (IIO_V1_LIB AND IIO_V1_INCLUDE))
    message(STATUS "libiio v1 not found — skipping iio-backend-v1")
    return()
endif()

file(GLOB V1_HEADERS include/v1/*.h)
file(GLOB V1_SOURCES src/*.cpp)

add_library(iio-backend-v1 SHARED
    ${V1_HEADERS}
    ${V1_SOURCES}
)

# Produce exactly libiio-backend-v1.so so the loader's
# dlopen("libiio-backend-v1.so") resolves it.
set_target_properties(iio-backend-v1 PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "iio-backend-v1"
)

target_include_directories(iio-backend-v1
    PRIVATE include include/v1 ${IIO_V1_INCLUDE}
)

target_link_libraries(iio-backend-v1
    PRIVATE Qt6::Core Qt6::Concurrent ${IIO_V1_LIB} iio-utils
)
