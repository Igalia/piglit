// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions GL_ARB_texture_cube_map_array
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require
#extension GL_ARB_texture_cube_map_array: require

uniform sampler2D s2D;

in int component_select;	/* not a const expr! */

void main()
{
	gl_Position = textureGather(s2D, vec2(0), component_select);
}
