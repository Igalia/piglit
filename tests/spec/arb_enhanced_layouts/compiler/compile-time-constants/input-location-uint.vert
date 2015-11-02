// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

const uint start = 3u;
layout(location = start + 2u) in vec4 b;

void main()
{
	gl_Position = b;
}
