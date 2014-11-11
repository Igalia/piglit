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
	gen_builtin_packing_tests.py
	templates/gen_builtin_packing_tests/vs_unpack.shader_test.mako
	templates/gen_builtin_packing_tests/vs_pack.shader_test.mako
	templates/gen_builtin_packing_tests/fs_unpack.shader_test.mako
	templates/gen_builtin_packing_tests/fs_pack.shader_test.mako
	templates/gen_builtin_packing_tests/const_pack.shader_test.mako
	templates/gen_builtin_packing_tests/const_unpack.shader_test.mako
	)
piglit_make_generated_tests(
	builtin_uniform_tests.list
	gen_builtin_uniform_tests.py
	builtin_function.py)
piglit_make_generated_tests(
	constant_array_size_tests.list
	gen_constant_array_size_tests.py
	builtin_function.py)
piglit_make_generated_tests(
	const_builtin_equal_tests.list
	gen_const_builtin_equal_tests.py
	templates/gen_const_builtin_equal_tests/template.shader_test.mako
	)
piglit_make_generated_tests(
	interpolation_tests.list
	gen_interpolation_tests.py)
piglit_make_generated_tests(
	non-lvalue_tests.list
	gen_non-lvalue_tests.py)
piglit_make_generated_tests(
	texture_query_lod_tests.list
	gen_texture_query_lod_tests.py
	templates/gen_texture_query_lod_tests/template.glsl_parser_test.mako
	)
piglit_make_generated_tests(
	shader_bit_encoding_tests.list
	gen_shader_bit_encoding_tests.py
	templates/gen_shader_bit_encoding_tests/template.shader_test.mako
	)
piglit_make_generated_tests(
	uniform-initializer_tests.list
	gen_uniform_initializer_tests.py
	templates/gen_uniform_initializer_tests/fs-initializer.shader_test.mako
	templates/gen_uniform_initializer_tests/vs-initializer.shader_test.mako
	templates/gen_uniform_initializer_tests/fs-initializer-from-const.shader_test.mako
	templates/gen_uniform_initializer_tests/vs-initializer-from-const.shader_test.mako
	templates/gen_uniform_initializer_tests/fs-initializer-set-by-API.shader_test.mako
	templates/gen_uniform_initializer_tests/vs-initializer-set-by-API.shader_test.mako
	templates/gen_uniform_initializer_tests/fs-initializer-set-by-other-stage.shader_test.mako
	templates/gen_uniform_initializer_tests/vs-initializer-set-by-other-stage.shader_test.mako
	)
piglit_make_generated_tests(
	builtin_cl_int_tests.list
	generate-cl-int-builtins.py)
piglit_make_generated_tests(
	cl_store_tests.list
	generate-cl-store-tests.py)
piglit_make_generated_tests(
	builtin_cl_math_tests.list
	generate-cl-math-builtins.py)
piglit_make_generated_tests(
	builtin_cl_relational_tests.list
	generate-cl-relational-builtins.py)
piglit_make_generated_tests(
	interpolation-qualifier-built-in-variable.list
	interpolation-qualifier-built-in-variable.py)
piglit_make_generated_tests(
	texture_lod_tests.list
	gen_texture_lod_tests.py
	templates/gen_texture_lod_tests/tex_grad.vert.mako
	templates/gen_texture_lod_tests/tex_grad.frag.mako
	templates/gen_texture_lod_tests/frag_lod.glsl_parser_test.mako
	)
piglit_make_generated_tests(
	outerproduct_invalid_params.list
	gen_outerproduct_invalid_params.py)
piglit_make_generated_tests(
	outerproduct_tests.list
	gen_outerproduct_tests.py
	templates/gen_outerproduct_tests/template.shader_test.mako
	)

piglit_make_generated_tests(
	builtin_uniform_tests_fp64.list
	gen_builtin_uniform_tests_fp64.py
	builtin_function_fp64.py)
piglit_make_generated_tests(
	constant_array_size_tests_fp64.list
	gen_constant_array_size_tests_fp64.py
	builtin_function_fp64.py)

# Add a "gen-tests" target that can be used to generate all the
# tests without doing any other compilation.
add_custom_target(gen-tests ALL
	DEPENDS builtin_packing_tests.list
		outerproduct_tests.list
		outerproduct_invalid_params.list
		builtin_uniform_tests.list
		constant_array_size_tests.list
		const_builtin_equal_tests.list
		builtin_cl_int_tests.list
		builtin_cl_math_tests.list
		builtin_cl_relational_tests.list
		cl_store_tests.list
		interpolation_tests.list
		non-lvalue_tests.list
		texture_query_lod_tests.list
		texture_lod_tests.list
		shader_bit_encoding_tests.list
		uniform-initializer_tests.list
		interpolation-qualifier-built-in-variable.list
		builtin_uniform_tests_fp64.list
		constant_array_size_tests_fp64.list)
