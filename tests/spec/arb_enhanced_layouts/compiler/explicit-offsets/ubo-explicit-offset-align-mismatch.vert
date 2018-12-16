// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
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
// Tests whether a block with conflicting offset and alignment requirements
// is accepted.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140) uniform block {
       layout(offset = 4, align = 16) float var1;
};

void main()
{
}
