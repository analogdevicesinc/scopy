function(create_test test_name source_file)
    add_executable(${test_name} ${source_file})

    add_test(NAME ${test_name} COMMAND ${test_name})

    target_link_libraries(
        ${test_name} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                             Qt${QT_VERSION_MAJOR}::Test scopy::gui
    )
endfunction()

create_test(run-single-widget-test run_single_widget_unittest.cpp)
create_test(spinbox-test spinbox_unittest.cpp)
