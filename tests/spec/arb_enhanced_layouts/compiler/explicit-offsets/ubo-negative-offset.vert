// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// ARB_enhanced_layouts spec says:
//    "The *offset* qualifier forces the qualified member to start at or after the
//    specified integral-constant-expression, which will be its byte offset
//    from the beginning of the buffer."
//
// Tests if negative offsets trigger a compile-time error.
// Note: not explicitly mentioned in the spec.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140) uniform block {
       layout(offset = -2) vec4 var1; // Wrong: offset cannot be negative value
       layout(offset = 0) vec4 var2;
};

void main()
{
}
