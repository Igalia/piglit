// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "It is a compile-time error to
//    specify an *offset* that is smaller than the offset of the previous
//    member in the block or that lies within the previous member of the
//    block."
//
// Tests whether choosing an offset that is larger than the previous element's
// size, but smaller than its base alignment, is accepted.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140) uniform block {
       layout(offset = 0) vec3 var1;
       layout(offset = 12) float var2;
};

void main()
{
}
