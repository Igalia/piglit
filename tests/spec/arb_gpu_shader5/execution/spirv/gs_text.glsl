#version 450

layout(points) in;
layout(points, max_vertices = 3) out;

layout(xfb_buffer = 0, xfb_stride = 4) out;
layout(xfb_buffer = 1, xfb_stride = 8) out;
layout(xfb_buffer = 2, xfb_stride = 20) out;

layout(stream = 0, location = 0, xfb_buffer = 0, xfb_offset = 0)
out float stream0_0_out;
layout(stream = 1, location = 1, xfb_buffer = 1, xfb_offset = 0)
out vec2 stream1_0_out;
layout(stream = 2, location = 2, xfb_buffer = 2, xfb_offset = 0)
out float stream2_0_out;
layout(stream = 2, location = 3, xfb_buffer = 2, xfb_offset = 4)
out vec4 stream2_1_out;

void main() {
  gl_Position = gl_in[0].gl_Position;
  stream0_0_out = 0.0;
  EmitStreamVertex(0);
  EndStreamPrimitive(0);

  stream2_0_out = 0.0;
  stream2_1_out = vec4(1.0, 2.0, 3.0, 4.0);
  EmitStreamVertex(2);
  EndStreamPrimitive(2);

  stream1_0_out = vec2(0.0, 1.0);
  EmitStreamVertex(1);
  EndStreamPrimitive(1);
}
