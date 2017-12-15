#version 450
#extension GL_ARB_enhanced_layouts: require

struct Array {
  float x2_out;
};
struct AoA {
  Array x2_Array[2];
};
struct S {
  float x1_out;
  AoA x2_AoA[2];
  float x3_out;
};
layout(location = 0, xfb_offset = 0) out S s1;
layout(location = 6, xfb_offset = 0, xfb_buffer = 2) out struct S2 {
  float y1_out;
  vec4 y2_out;
} s2;
void main() {
  gl_Position = vec4(0.0);
  s1.x1_out = 1.0;
  s1.x2_AoA[0].x2_Array[0].x2_out = 2.0;
  s1.x2_AoA[0].x2_Array[1].x2_out = 3.0;
  s1.x2_AoA[1].x2_Array[0].x2_out = 4.0;
  s1.x2_AoA[1].x2_Array[1].x2_out = 5.0;
  s1.x3_out = 6.0;
  s2.y1_out = 7.0;
  s2.y2_out = vec4(8.0, 9.0, 10.0, 11.0);
}

