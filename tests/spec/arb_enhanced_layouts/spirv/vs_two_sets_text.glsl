#version 450
#extension GL_ARB_enhanced_layouts: require

layout(location=0, xfb_offset = 0) out float x1_out;
layout(location=1, xfb_offset = 4) out float x2_out[2];
layout(location=3, xfb_offset = 12) out vec3 x3_out;
layout(xfb_buffer = 2) out;
layout(location=5, xfb_offset = 0, xfb_buffer = 2) out float y1_out;
layout(location=6, xfb_offset = 4) out vec4 y2_out;
void main() {
  gl_Position = vec4(0.0);
  x1_out = 1.0;
  x2_out[0] = 2.0;
  x2_out[1] = 3.0;
  x3_out = vec3(4.0, 5.0, 6.0);
  y1_out = 7.0;
  y2_out = vec4(8.0, 9.0, 10.0, 11.0);
}

