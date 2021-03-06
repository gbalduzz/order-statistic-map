function (maplib_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} gtest gtest_main pthread)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

maplib_add_test(order_statistic_map_test)
maplib_add_test(order_statistic_set_test)
maplib_add_test(sampling_map_test)
maplib_add_test(sampling_set_test)

option(BUILD_PERFORMANCE_TEST "Build performance test. Requires google benchmark library." OFF)

if(BUILD_PERFORMANCE_TEST)
    function (maplib_add_perftest name)
        add_executable(${name} ${name}.cpp)
        target_link_libraries(${name} benchmark_main benchmark pthread)
        # add_test(NAME ${name} COMMAND ${name})
    endfunction()

    maplib_add_perftest(order_statistic_map_small_data_perftest)
    maplib_add_perftest(order_statistic_map_big_data_perftest)
    maplib_add_perftest(order_statistic_map_string_perftest)
    maplib_add_perftest(sampling_map_perftest)
endif()


# Copy auxiliary files for plotting
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/plot.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/generate_all_plots.bash
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
