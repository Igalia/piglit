// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

/* From the GL_ARB_shader_storage_buffer_object:
 *
 *     "Layout qualifiers on member declarations cannot use the shared,
 *      packed, std140, or std430 qualifiers."
 */

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

buffer ssbo {
	layout(std430) vec4 a;
};

vec4 foo(void) {
	return a;
}
