// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_explicit_attrib_location GL_ARB_enhanced_layouts
// [end config]

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

const int start = 0;
layout(location = 0, index = start + 1) out vec4 color;

void main()
{
	color = vec4(1.0);
}
