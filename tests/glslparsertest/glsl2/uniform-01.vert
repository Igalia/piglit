/* PASS */
#version 120
uniform vec2 a = vec2(1.0, 2.0);

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
