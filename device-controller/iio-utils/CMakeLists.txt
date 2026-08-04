# iio-utils — libiio backend interfaces (I*Ops), opaque handles, concrete commands,
# the dlopen backend loader, and the scan helper. Depends on core; no direct libiio.
file(GLOB IIOUTIL_HEADERS
    include/iioutil/*.h
    include/iioutil/commands/*.h
)
file(GLOB IIOUTIL_SOURCES src/*.cpp)

add_library(controller-iioutils SHARED
    ${IIOUTIL_HEADERS}
    ${IIOUTIL_SOURCES}
)

target_include_directories(controller-iioutils
    PUBLIC include
)

target_link_libraries(controller-iioutils
    PUBLIC controller-core Qt6::Core
    PRIVATE ${CMAKE_DL_LIBS}
)

if(DC_STANDALONE)
    add_subdirectory(test)
endif()
