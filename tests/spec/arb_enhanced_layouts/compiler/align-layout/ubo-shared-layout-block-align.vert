// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The *align* qualifier can only be used on blocks or block members, and
//    only for blocks declared with *std140* or *std430* layouts."
//
// Tests for compiler error, when the block is of shared layout.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(shared, align = 32) uniform block {
	vec4 var1;
	vec4 var2;
};

void main()
{
}
