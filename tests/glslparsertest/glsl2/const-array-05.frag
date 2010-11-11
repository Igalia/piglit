// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

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
