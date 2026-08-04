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
)

if(DC_STANDALONE)
    add_subdirectory(test)
endif()
