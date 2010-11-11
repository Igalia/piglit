// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS */
#version 110
#extension GL_ARB_draw_buffers: disable

uniform vec4 a;

void main()
{
  gl_FragData[0] = a;
}
