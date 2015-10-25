// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "It is a compile-time error to
//    specify an *offset* that is smaller than the offset of the previous
//    member in the block..."
//
// Tests whether assigning a smaller offset for sequential member triggers
// a compile-time error.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140) uniform block {
       layout(offset = 32) vec4 var1;
       layout(offset = 0) vec4 var2; // Wrong: offset must be larger than that of a previous member
};

void main()
{
}
