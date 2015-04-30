// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *     "Buffer variables cannot have initializers"
 */

buffer a {
	vec4 b = vec4(1.0);
};

vec4 foo(void) {
	return b;
}
