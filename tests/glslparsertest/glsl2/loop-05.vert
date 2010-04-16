/* FAIL - loop body does not start a new scope */

void main()
{
  while (bool i = true)
    float i = gl_Vertex.x;

  gl_Position = gl_Vertex;
}
