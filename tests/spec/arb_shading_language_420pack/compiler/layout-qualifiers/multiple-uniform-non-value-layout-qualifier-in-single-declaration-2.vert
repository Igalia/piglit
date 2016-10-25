// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_shading_language_420pack GL_ARB_enhanced_layouts
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration."
//
// ARB_enhanced_layouts spec says:
//
//    "The *align* qualifier can only be used on blocks or block
//     members, and only for blocks declared with *std140* or *std430*
//     layouts."

#version 140
#extension GL_ARB_shading_language_420pack: enable
#extension GL_ARB_enhanced_layouts : enable

layout(row_major) layout(std140) uniform;

layout(align = 32) uniform block {
	vec4 var1;
	vec4 var2;
};

void main()
{
}
