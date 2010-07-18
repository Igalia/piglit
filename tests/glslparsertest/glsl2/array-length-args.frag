/* FAIL: The length() method takes no arguments. */
#version 120
void main()
{
   float a[5];
   if (a.length(1337) == 5)
      gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
   else
      gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
