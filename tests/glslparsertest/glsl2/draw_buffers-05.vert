/* FAIL - GL_ARB_draw_buffers does not exist in the vertex shader. */
#version 110
#extension GL_ARB_draw_buffers: require

uniform vec4 a;

void main()
{
  gl_Position = a;
}
