// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_shader_storage_buffer_object
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "It is a compile-time error to
//    specify an *offset* that is smaller than the offset of the previous
//    member in the block or that lies within the previous member of the
//    block."
//
// Tests whether assigning the same offsets for multiple members trigger
// a compile-time error.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std430) buffer b {
       layout(offset = 0) vec4 var1;
       layout(offset = 8) vec4 var2; // Wrong: must use 16+ as offset
};

void main()
{
}
