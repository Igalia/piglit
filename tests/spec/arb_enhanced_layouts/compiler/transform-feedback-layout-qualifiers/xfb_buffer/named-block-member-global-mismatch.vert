// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "Shaders in the transform feedback capturing mode have an initial global
//    default of
//
//        layout(xfb_buffer = 0) out;"
//
//    ...
//
//    "When a variable or output block is declared without an xfb_buffer
//    qualifier, it inherits the global default buffer. When a variable or
//    output block is declared with an xfb_buffer qualifier, it has that
//    declared buffer. All members of a block inherit the block's buffer. A
//    member is allowed to declare an xfb_buffer, but it must match the buffer
//    inherited from its block, or a compile-time error results."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout (xfb_buffer = 1) out;

out block {
  vec4 var1;
  layout(xfb_buffer = 0) vec4 var2; // xfb_buffer must be 1
} block_name;

void main()
{
}
