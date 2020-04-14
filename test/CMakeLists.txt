function (ramlib_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} gtest gtest_main pthread)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

ramlib_add_test(random_access_map_test)
ramlib_add_test(random_access_set_test)

option(BUILD_PERFORMANCE_TEST "Build performance test. Requires google benchmark library." OFF)

if(BUILD_PERFORMANCE_TEST)
    function (ramlib_add_perftest name)
        add_executable(${name} ${name}.cpp)
        target_link_libraries(${name} benchmark_main benchmark pthread)
        # add_test(NAME ${name} COMMAND ${name})
    endfunction()

ramlib_add_perftest(random_access_map_perftest)
endif()
