// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The *align* qualifier can only be used on blocks or block members, and
//    only for blocks declared with *std140* or *std430* layouts."
//
// Tests for successful compilation, when the block is of std140 layout.
//

#version 150
#extension GL_ARB_enhanced_layouts : enable

layout(std140) uniform block {
	layout(align = 32) vec4 var1;
	layout(align = 16) vec4 var2;
} named;

void main()
{
}
