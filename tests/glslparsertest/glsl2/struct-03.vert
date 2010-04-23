/* PASS */

vec4 foo(vec4 a, vec4 b)
{
  return vec4(dot(a, b));
}

void main()
{
  struct foo {
    float f;
    int i;
    bool b;
  };

  gl_Position = gl_Vertex;
}
