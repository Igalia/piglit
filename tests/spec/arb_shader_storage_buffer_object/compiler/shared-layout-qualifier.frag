// [config]
// expect_result: pass
// glsl_version: 1.30
// require_extensions: GL_ARB_shader_storage_buffer_object GL_ARB_uniform_buffer_object GL_ARB_compute_shader
// [end config]

#version 130
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_ARB_uniform_buffer_object: require
#extension GL_ARB_compute_shader: require

layout (shared) buffer ssbo {
	float a;
};

void foo(void) {
	a = 1.0;
}
