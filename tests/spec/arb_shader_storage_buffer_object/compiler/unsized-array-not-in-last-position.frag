// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *     "In a shader storage block, the last member may be declared without an
 *     explicit size."
 */

buffer a {
	vec4 b;
	int c[];
	float d;
};

vec4 foo(void) {
	return b;
}
