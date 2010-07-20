#version 120
/* PASS - implicit conversions are done in array constructors */
void main()
{
  float a[];

  a = float[](4, 5);

  gl_Position = gl_Vertex;
}
