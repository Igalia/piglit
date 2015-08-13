// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 330
#extension GL_ARB_shader_storage_buffer_object: require
coherent restrict volatile buffer Buffer {
	vec4 foo;
} buf;
vec4 foo(void) { return buf.foo; }
