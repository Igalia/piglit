// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - no implicit conversions for return values */
vec2 foo()
{
   float x = 2.0;
   return x;
}
void main()
{
   gl_FragColor = vec4(0.0, foo(), 0.0);
}
