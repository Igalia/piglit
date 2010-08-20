/* FAIL - variables and functions share the same namespace */
const float f = 1.0;

float f(float x)
{
  return pow(x, 2.1718281828);
}

void main()
{
  gl_Position = vec4(0.0);
}
