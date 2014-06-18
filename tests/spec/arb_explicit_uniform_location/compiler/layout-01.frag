// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_explicit_uniform_location
// [end config]
//

#version 330
#extension GL_ARB_explicit_uniform_location: require
vec4 color;
layout(location = 42) uniform float test1;
uniform float test2;
uniform float test3;

void main()
{
	color = vec4(1.0);
}
