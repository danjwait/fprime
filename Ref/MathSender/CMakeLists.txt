# register the standard build
set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MathSender.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MathSender.fpp"
)
register_fprime_module()

# register the unit test build
set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MathSender.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
)
register_fprime_ut()