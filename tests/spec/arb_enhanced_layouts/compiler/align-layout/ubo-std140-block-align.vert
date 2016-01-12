// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The *align* qualifier can only be used on blocks or block members, and
//    only for blocks declared with *std140* or *std430* layouts."
//
// Tests for successful compilation, when align is used a the block level.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140, align = 32) uniform block {
	vec4 var1;
	layout(align = 16) vec4 var2;
};

void main()
{
}
