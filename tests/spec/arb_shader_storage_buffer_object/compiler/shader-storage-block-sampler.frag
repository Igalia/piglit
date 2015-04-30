// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *  "The "buffer" qualifier can be used with any of the basic data types, or
 *   when declaring a variable whose type is a structure, or an array of any of
 *   these."
 */

buffer a {
	sampler2D s;
};

vec4 foo(void) {
	return texture2D(s, vec2(0.0));
}
