// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions GL_ARB_texture_cube_map_array
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require
#extension GL_ARB_texture_cube_map_array: require

uniform sampler2DShadow s2D;
uniform sampler2DArrayShadow s2DArray;
uniform samplerCubeShadow sCube;
uniform samplerCubeArrayShadow sCubeArray;
uniform sampler2DRectShadow s2DRect;

void main()
{
	vec4 res = vec4(0);

	float refz = 0.5;

	res += textureGather(s2D,		vec2(0), refz);
	res += textureGather(s2DArray,		vec3(0), refz);
	res += textureGather(sCube,		vec3(0), refz);
	res += textureGather(sCubeArray,	vec4(0), refz);
	res += textureGather(s2DRect,		vec2(0), refz);

	gl_Position = res;
}
