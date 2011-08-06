# Execute gen_builtin_uniform_tests.py once during configure to find
# out what files it generates.
set(gen_builtin_uniform
	${CMAKE_CURRENT_SOURCE_DIR}/gen_builtin_uniform_tests.py)
execute_process(
	COMMAND ${python} ${gen_builtin_uniform} --names-only
        OUTPUT_VARIABLE builtin_uniform_tests
	RESULT_VARIABLE builtin_uniform_tests_result)
if(NOT builtin_uniform_tests_result EQUAL 0)
	message(FATAL_ERROR "gen_builtin_uniform_tests.py failed")
endif(NOT builtin_uniform_tests_result EQUAL 0)
string(REPLACE "\n" ";" builtin_uniform_tests ${builtin_uniform_tests})

# Add a custom command which executes gen_builtin_uniform_tests.py
# during the build.
add_custom_command(OUTPUT ${builtin_uniform_tests}
	COMMAND ${python} ${gen_builtin_uniform}
	DEPENDS gen_builtin_uniform_tests.py builtin_function.py
	VERBATIM)

# And add a "gen-tests" target that can be used to generate all the
# tests without doing any other compilation.
add_custom_target(gen-tests ALL
	DEPENDS ${builtin_uniform_tests})
