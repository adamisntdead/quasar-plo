add_executable(test_version test_version.cpp)
target_link_libraries(test_version PRIVATE quasar_engine)
add_test(NAME test_version COMMAND test_version)

add_executable(test_plo_legal test_plo_legal.cpp)
target_link_libraries(test_plo_legal PRIVATE quasar_engine)
add_test(NAME test_plo_legal COMMAND test_plo_legal)
