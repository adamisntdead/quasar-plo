add_executable(test_version test_version.cpp)
target_link_libraries(test_version PRIVATE quasar_engine)
add_test(NAME test_version COMMAND test_version)

add_executable(test_plo_legal test_plo_legal.cpp)
target_link_libraries(test_plo_legal PRIVATE quasar_engine)
add_test(NAME test_plo_legal COMMAND test_plo_legal)

# CLI golden-ish test: run CLI and expect certain JSON keys/values
add_test(NAME test_cli_legal
         COMMAND $<TARGET_FILE:quasar_cli> ${CMAKE_SOURCE_DIR}/scripts/example_spot.json)
set_tests_properties(test_cli_legal PROPERTIES
  PASS_REGULAR_EXPRESSION "\"can_check\":false;\"call_amount\":1;\"raise\":\{\"min_to\":4"
)
