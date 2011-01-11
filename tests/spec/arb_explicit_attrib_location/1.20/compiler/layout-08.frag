// [config]
// expect_result: pass
// glsl_version: 1.20
// extension: GL_ARB_explicit_attrib_location
// [end config]

#version 120
#extension GL_ARB_explicit_attrib_location: require
layout(location = 0X01) out vec4 color;

void main()
{
	color = vec4(1.0);
}
