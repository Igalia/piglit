// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

uniform sampler2D s2D;
uniform ivec2[4] offsets;

void main()
{
	gl_Position = textureGatherOffsets(s2D, vec2(0), offsets);
}
