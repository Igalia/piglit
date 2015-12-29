// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_shader_storage_buffer_object
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//
//    "The specified offset must be a multiple of the base alignment of the
//    type of the block member it qualifies, or a compile-time error results."
//

#version 140
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std430) buffer b {
       layout(offset = 8) vec4 var1;
       layout(offset = 24) vec4 var2;
};

void main()
{
}
