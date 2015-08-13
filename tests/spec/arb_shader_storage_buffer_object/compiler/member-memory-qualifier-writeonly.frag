// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

buffer ssbo {
	writeonly float a;
};

float foo(void) {
	return a;
}
