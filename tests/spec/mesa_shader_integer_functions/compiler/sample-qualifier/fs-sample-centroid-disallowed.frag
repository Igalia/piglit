// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

/* At most one aux. storage qualifier is allowed.
 * Note that strictly speaking, under gpu_shader5 / GLSL 4.00,
 * the storage qualifier is the entire 'centroid in' or 'sample in',
 * but we use the wording from shading_language_420pack / GLSL 4.20.
 *
 * A strict gpu_shader5 / GLSL 4.00 implementation should reject this
 * too.
 */
sample centroid in vec4 x;
out vec4 out_color;

void main()
{
	out_color = x;
}

