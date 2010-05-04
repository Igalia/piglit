#version 120
/* PASS */
uniform vec4 [3] a;

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
