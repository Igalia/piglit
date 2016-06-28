// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions GL_ARB_texture_cube_map_array
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require
#extension GL_ARB_texture_cube_map_array: require

uniform isampler2D s2D;
uniform isampler2DArray s2DArray;
uniform isamplerCube sCube;
uniform isamplerCubeArray sCubeArray;
uniform isampler2DRect s2DRect;

void main()
{
	ivec4 res = ivec4(0);

	res += textureGather(s2D,		vec2(0), 0);
	res += textureGather(s2DArray,		vec3(0), 1);
	res += textureGather(sCube,		vec3(0), 2);
	res += textureGather(sCubeArray,	vec4(0), 3);
	res += textureGather(s2DRect,		vec2(0), 0);

	gl_FragColor = vec4(res);
}
