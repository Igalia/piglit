# Add a custom command which executes gen_builtin_uniform_tests.py
# during the build.
add_custom_command(
	OUTPUT builtin_uniform_tests.list
	COMMAND ${python} ${CMAKE_CURRENT_SOURCE_DIR}/gen_builtin_uniform_tests.py > builtin_uniform_tests.list
	DEPENDS gen_builtin_uniform_tests.py builtin_function.py
	VERBATIM)

# Add a "gen-tests" target that can be used to generate all the tests
# without doing any other compilation.
add_custom_target(gen-tests ALL
	DEPENDS builtin_uniform_tests.list)
