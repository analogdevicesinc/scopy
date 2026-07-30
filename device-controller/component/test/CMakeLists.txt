find_package(Qt6 REQUIRED COMPONENTS Test)

macro(add_component_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE component core iio-utils Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endmacro()

add_component_test(tst_component_attribute)
add_component_test(tst_component_controller)
add_component_test(tst_component_stream)
add_component_test(tst_component_register)
add_component_test(tst_component_trigger)
