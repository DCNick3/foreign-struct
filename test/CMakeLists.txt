
include(ForeignTests)

function(foreign_test_suite)
    cmake_parse_arguments(
            ""
            ""
            "SUITE_NAME"
            ""
            ${ARGN}
    )

    add_executable(foreign_${_SUITE_NAME}_ut
            main.cpp
            ${_SUITE_NAME}.cpp)
    target_link_libraries(foreign_${_SUITE_NAME}_ut
            ut foreign_struct cxx-sized-int-literals cxxopts)
    foreign_discover_tests(foreign_${_SUITE_NAME}_ut TEST_PREFIX ${_SUITE_NAME}-)
endfunction()

foreign_test_suite(SUITE_NAME integer)
foreign_test_suite(SUITE_NAME enum)
foreign_test_suite(SUITE_NAME array)
foreign_test_suite(SUITE_NAME struct)
foreign_test_suite(SUITE_NAME union)
