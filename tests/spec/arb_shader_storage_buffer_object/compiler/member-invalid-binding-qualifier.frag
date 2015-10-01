// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *  "It is an error to specify the binding identifier for the global
 *  scope or for block member declarations.
 */

#version 150
#extension GL_ARB_shader_storage_buffer_object: require

buffer buf {
	layout(binding=42) float f;
};

float foo(void) {
	return f;
}
