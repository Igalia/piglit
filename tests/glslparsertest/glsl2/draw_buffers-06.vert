// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - GL_ARB_draw_buffers does exist in the vertex shader, but it only
 * makes the built in variable gl_MaxDrawBuffers be available.
 */
#version 110

void main()
{
  gl_Position = vec4(gl_MaxDrawBuffers);
}
