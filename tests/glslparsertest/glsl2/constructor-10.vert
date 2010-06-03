/* FAIL - not enough data provided for construction */

vec4 foo()
{
  return vec4();
}

void main()
{
  gl_Position = foo();
}
