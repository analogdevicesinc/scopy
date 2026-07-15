# core — Result, Command, executors, BufferUser. No backend/libiio types, no Qt widgets.
file(GLOB CORE_HEADERS include/core/*.h)
file(GLOB CORE_SOURCES src/*.cpp)

add_library(core SHARED
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

target_include_directories(core
    PUBLIC include
)

target_link_libraries(core
    PUBLIC Qt6::Core Qt6::Concurrent QCoro6::Core QCoro6::Coro
)

add_subdirectory(test)
