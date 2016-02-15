// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]

#version 150
#extension GL_ARB_enhanced_layouts: require

layout (xfb_buffer = 2) out;

layout(xfb_buffer = 1) out block {
  vec4 var1;
  layout(xfb_buffer = 1) vec4 var2;
};

out block2 {
  vec4 var1;
  layout(xfb_buffer = 2) vec4 var2;
} block_name;

layout (xfb_buffer = 3) out;

layout (xfb_buffer = 0) out vec4 var5;

void main()
{
}
