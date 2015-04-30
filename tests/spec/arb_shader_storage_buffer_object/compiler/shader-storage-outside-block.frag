// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *  "It is illegal to declare buffer variables at global scope
 *   (outside a block)."
 */

buffer vec4 a;

vec4 foo(void) {
	return a;
}
