attribute mat4 color;
attribute vec4 normalization;

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  gl_FrontColor = color[IDX] * normalization;
}
