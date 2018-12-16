// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_shader_storage_buffer_object
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The *align* qualifier, when used on a block, has the same effect as
//    qualifying each member with the same *align* value as declared on the
//    block, and gets the same compile-time results and errors as if this had
//    been done.  As described in general earlier, an individual member can
//    specify its own *align*, which overrides the block-level *align*, but
//    just for that member."
//
// Tests whether a block member with an explicit alignment requirement
// overriding the block-level alignment succeeds.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std430, align = 16) buffer b {
       layout(offset = 8, align = 8) vec2 var1;
       layout(offset = 16) float var2; // would be inside `var1` without align = 8
};

void main()
{
}
