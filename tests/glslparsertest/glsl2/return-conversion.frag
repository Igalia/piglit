/* FAIL - no implicit conversions for return values */
float foo()
{
   int x = 1;
   return x;
}
void main()
{
   gl_FragColor = vec4(0.0, foo(), 0.0, 0.0);
}
