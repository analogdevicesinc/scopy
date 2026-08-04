# core — Result, Command, executors, BufferUser. No backend/libiio types, no Qt widgets.
file(GLOB CORE_HEADERS include/core/*.h)
file(GLOB CORE_SOURCES src/*.cpp)

add_library(controller-core SHARED
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

target_include_directories(controller-core
    PUBLIC include
)

target_link_libraries(controller-core
    PUBLIC Qt6::Core Qt6::Concurrent QCoro6::Core QCoro6::Coro
)

if(DC_STANDALONE)
    add_subdirectory(test)
endif()
