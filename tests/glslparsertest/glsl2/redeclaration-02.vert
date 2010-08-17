/* FAIL - variables and functions share the same namespace */
const float exp = 1.0;

float exp(float x)
{
  return pow(x, 2.1718281828);
}

void main()
{
  const float exp = 2.0;
  gl_Position = vec4(0.0);
}
