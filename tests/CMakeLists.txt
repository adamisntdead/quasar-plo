add_executable(test_version tests_main.cpp test_version.cpp)
target_link_libraries(test_version PRIVATE quasar_engine)
add_test(NAME test_version COMMAND test_version)

