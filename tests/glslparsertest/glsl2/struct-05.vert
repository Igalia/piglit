/* FAIL - structure name conflicts with variable name in same scope */

uniform vec4 foo;

struct foo {
  float f;
  int i;
  bool b;
};

void main()
{
  foo foo;

  gl_Position = gl_Vertex;
}
