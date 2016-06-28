// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions GL_ARB_texture_cube_map_array
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require
#extension GL_ARB_texture_cube_map_array: require

uniform sampler2D s2D;
uniform sampler2DArray s2DArray;
uniform samplerCube sCube;
uniform samplerCubeArray sCubeArray;
uniform sampler2DRect s2DRect;

void main()
{
	vec4 res = vec4(0);

	res += textureGather(s2D,		vec2(0));
	res += textureGather(s2DArray,		vec3(0));
	res += textureGather(sCube,		vec3(0));
	res += textureGather(sCubeArray,	vec4(0));
	res += textureGather(s2DRect,		vec2(0));

	gl_Position = res;
}
