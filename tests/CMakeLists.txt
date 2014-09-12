add_executable( test_model test_model.c )

target_link_libraries( test_model sgui )

set_target_properties( test_model PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests" )

add_test( NAME sgui_model COMMAND test_model )

