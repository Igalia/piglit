// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

uniform sampler2DShadow s2D;
uniform sampler2DArrayShadow s2DArray;
uniform sampler2DRectShadow s2DRect;

void main()
{
	vec4 res = vec4(0);

	float refz = 0.5;
	ivec2 offset = ivec2(-8, 7);

	res += textureGatherOffset(s2D,		vec2(0), refz, offset);
	res += textureGatherOffset(s2DArray,	vec3(0), refz, offset);
	res += textureGatherOffset(s2DRect,	vec2(0), refz, offset);

	gl_Position = res;
}
