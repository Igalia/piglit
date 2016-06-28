// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

uniform usampler2D s2D;
uniform usampler2DArray s2DArray;
uniform usampler2DRect s2DRect;

const ivec2 offset = ivec2(-8, 7);

void main()
{
	uvec4 res = uvec4(0);

	res += textureGatherOffset(s2D,		vec2(0), offset, 0);
	res += textureGatherOffset(s2DArray,	vec3(0), offset, 1);
	res += textureGatherOffset(s2DRect,	vec2(0), offset, 2);

	gl_Position = vec4(res);
}
