// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 330
#extension GL_ARB_shader_storage_buffer_object: require
readonly buffer Buffer {
	float foo;
} buf;

void main() {
	buf.foo = 1.0;
}
