#version 450
#extension GL_ARB_enhanced_layouts: require

out block {
  layout(location = 0, xfb_offset = 0) out float x1_out;
  layout(location = 1, xfb_offset = 4) out vec2 x2_out;
  layout(location = 2, xfb_buffer = 0) out vec3 not_captured;
  layout(location = 3, xfb_offset = 12) out vec3 x3_out;
} x;
layout(xfb_buffer = 2) out;
layout(location = 4, xfb_offset = 0) out block2 {
  float y1_out;
  vec4 y2_out;
} y;
void main() {
  gl_Position = vec4(0.0);
  x.x1_out = 1.0;
  x.x2_out = vec2(2.0, 3.0);
  x.x3_out = vec3(4.0, 5.0, 6.0);
  y.y1_out = 7.0;
  y.y2_out = vec4(8.0, 9.0, 10.0, 11.0);
}

