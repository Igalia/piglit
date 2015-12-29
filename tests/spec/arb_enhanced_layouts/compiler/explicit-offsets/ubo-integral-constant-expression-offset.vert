// [config]
// expect_result: pass
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
// Tests if constant expressions are accepted as offset.
//

#version 140
#extension GL_ARB_enhanced_layouts : enable

const int start = 16;

layout(std140) uniform block {
       layout(offset = start + 0) vec4 var1;
       layout(offset = start + 32) vec4 var2;
};

void main()
{
}
