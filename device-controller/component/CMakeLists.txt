# component — identity/capability component tree (Context/Device/Channel/Attribute),
# the Controller + backend context factories, and the IIO backend leaves.
# Depends on core and iio-utils.
file(GLOB COMPONENT_HEADERS
    include/component/*.h
    include/component/backends/iio/*.h
)
file(GLOB COMPONENT_SOURCES
    src/*.cpp
    src/backends/iio/*.cpp
)

add_library(component SHARED
    ${COMPONENT_HEADERS}
    ${COMPONENT_SOURCES}
)

target_include_directories(component
    PUBLIC include
)

target_link_libraries(component
    PUBLIC controller-core controller-iioutils Qt6::Core QCoro6::Core QCoro6::Coro
    PRIVATE iio-backend-v0
)

# Direct-link libiio v0: IIOContextFactory instantiates the v0 backend via the
# statically-linked createIIOBackend() instead of dlopen'ing a plugin through
# IIOBackendLoader (which stays compiled but dormant). See iiocontextfactory.cpp.
target_compile_definitions(component PRIVATE SCOPY_DC_DIRECT_LIBIIO_V0)
