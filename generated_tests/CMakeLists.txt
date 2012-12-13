# Create a custom command that runs the Python script
# ${generator_script} to generate tests.
#
# A list of the files generated will be output to the file
# ${file_list}.  This can be used to run the custom command by itself,
# and is also used by the build system to tell when the files need to
# be rebuilt.
#
# The custom command will automatically depend on ${generator_script}.
# Additional dependencies can be supplied using additional arguments.
function(piglit_make_generated_tests file_list generator_script)
	# Add a custom command which executes ${generator_script}
	# during the build.
	add_custom_command(
		OUTPUT ${file_list}
		COMMAND ${python} ${CMAKE_CURRENT_SOURCE_DIR}/${generator_script} > ${file_list}
		DEPENDS ${generator_script} ${ARGN}
		VERBATIM)
endfunction(piglit_make_generated_tests custom_target generator_script)

# Create custom commands and targets to build generated tests.
piglit_make_generated_tests(
	builtin_packing_tests.list
	gen_builtin_packing_tests.py)
piglit_make_generated_tests(
	builtin_uniform_tests.list
	gen_builtin_uniform_tests.py
	builtin_function.py)
piglit_make_generated_tests(
	constant_array_size_tests.list
	gen_constant_array_size_tests.py
	builtin_function.py)
piglit_make_generated_tests(
	interpolation_tests.list
	gen_interpolation_tests.py)
piglit_make_generated_tests(
	non-lvalue_tests.list
	gen_non-lvalue_tests.py)
piglit_make_generated_tests(
	uniform-initializer_tests.list
	gen_uniform_initializer_tests.py
	uniform-initializer-templates/fs-initializer.template
	uniform-initializer-templates/vs-initializer.template
	uniform-initializer-templates/fs-initializer-from-const.template
	uniform-initializer-templates/vs-initializer-from-const.template
	uniform-initializer-templates/fs-initializer-set-by-API.template
	uniform-initializer-templates/vs-initializer-set-by-API.template
	uniform-initializer-templates/fs-initializer-set-by-other-stage.template
	uniform-initializer-templates/vs-initializer-set-by-other-stage.template
	)

# Add a "gen-tests" target that can be used to generate all the
# tests without doing any other compilation.
add_custom_target(gen-tests ALL
	DEPENDS builtin_packing_tests.list
		builtin_uniform_tests.list
		constant_array_size_tests.list
		interpolation_tests.list
		non-lvalue_tests.list
		uniform-initializer_tests.list)
