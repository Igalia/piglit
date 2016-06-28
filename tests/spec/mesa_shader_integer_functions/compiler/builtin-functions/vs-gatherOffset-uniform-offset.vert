// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

uniform sampler2D s2D;
uniform ivec2 offset;		/* MESA_shader_integer_functions allows this to be uniform
				   rather than constexpr */

void main()
{
	vec4 res = vec4(0);

	res += textureGatherOffset(s2D,	vec2(0), offset);

	gl_Position = res;
}
