/* FAIL - gl_FragData requires that GL_ARB_draw_buffers be enabled. */
#version 110

uniform vec4 a;

void main()
{
  gl_FragData[0] = a;
}
