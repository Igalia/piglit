// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_shader_storage_buffer_object
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The /actual alignment/ of a member will be the greater of the specified
//    *align* alignment and the standard (e.g., *std140*) base alignment for the
//    member's type.  The /actual offset/ of a member is computed as follows:
//    If *offset* was declared, start with that offset, otherwise start with the
//    next available offset.  If the resulting offset is not a multiple of the
//    /actual alignment/, increase it to the first offset that is a multiple of
//    the /actual alignment/.  This results in the /actual offset/ the member
//    will have."
//
//    "It is a compile-time error to
//    specify an *offset* that is smaller than the offset of the previous
//    member in the block or that lies within the previous member of the
//    block."
//
// Tests whether a block with conflicting offset and alignment requirements
// followed by a field with an explicit offset that lies within the actual
// position of the previous member fails.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std430) buffer b {
       layout(offset = 8, align = 16) vec2 var1; // starts at actual offset 16
       layout(offset = 20) float var2; // error: inside var1
};

void main()
{
}
