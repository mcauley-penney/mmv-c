
add_executable(test_utils ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.c)
target_link_libraries(test_utils PRIVATE unity::framework)
add_test(NAME Utils COMMAND test_utils)

add_executable(test_set ${CMAKE_CURRENT_SOURCE_DIR}/test_set.c)
target_link_libraries(test_set PRIVATE unity::framework)
add_test(NAME Set COMMAND test_set)

add_executable(test_mmv ${CMAKE_CURRENT_SOURCE_DIR}/test_mmv.c)
target_link_libraries(test_mmv PRIVATE unity::framework)
add_test(NAME mmv COMMAND test_mmv)
set_tests_properties(mmv PROPERTIES ENVIRONMENT "EDITOR=true")
