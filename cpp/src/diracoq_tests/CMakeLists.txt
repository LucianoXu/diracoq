#################################
# Tests list
set(tests
    test_deBruijn
    test_calculus
    test_reduction
    test_special_eq
    test_trace2coq
    test_prover
)

foreach(test ${tests})
    add_executable(${test} ${test}.cpp)

    target_link_libraries(
        ${test}
        gtest_main DIRACOQ
    )

    add_test(
        NAME ${test}
        COMMAND ${test}
    )
endforeach()
