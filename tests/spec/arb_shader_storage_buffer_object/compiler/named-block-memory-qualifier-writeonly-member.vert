// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 330
#extension GL_ARB_shader_storage_buffer_object: require
buffer Buffer {
	writeonly float foo;
} buf;

void main(void) {
	float result = buf.foo;
}
