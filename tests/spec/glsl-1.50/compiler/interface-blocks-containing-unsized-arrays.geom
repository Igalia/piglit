// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: false
// [end config]
//
// Test that an interface block may contain members which are unsized
// arrays.  Both GLSL 1.10 and GLSL 1.20 style array declarations are
// tested.
//
// We cannot check linking because with geometry shaders, piglit adds
// a dummy vertex shader and in this case, for linking to work, we need
// that dummy vertex shader to include a matching output block, which
// it doesn't.

#version 150
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in block {
  float foo[];
  float[] bar;
} inst_in[];

out block {
  float foo[];
  float[] bar;
} inst_out;

void main()
{
}
