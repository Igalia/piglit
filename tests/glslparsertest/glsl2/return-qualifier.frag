// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - return types cannot have qualifiers */
const float one()
{
   return 1.0;
}
void main()
{
   gl_FragColor = vec4(0.0, one(), 0.0, 0.0);
}
