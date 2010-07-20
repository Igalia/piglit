/* PASS */
#version 120
void main()
{
   const float a[2] = float[2](0.0, 1.0);
   const float length = a.length();

   if (length == 2)
      gl_FragColor = vec4(a[0], a[1], a[0], a[1]);
   else
      gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
