find_package(Qt6 REQUIRED COMPONENTS Test)

macro(add_core_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE controller-core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endmacro()

add_core_test(tst_result)
add_core_test(tst_serialcmdexecutor)
add_core_test(tst_concurrentcmdexecutor)
