/* PASS */
uniform vec4 a[3];

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
