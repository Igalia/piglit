// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL: The length() method was added in GLSL 1.20. */
void main()
{
   float a[5];
   if (a.length() == 5)
      gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
   else
      gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
