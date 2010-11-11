// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* The program should terminate with an error message and not get into an
   infinite loop */
#ifdef name

void main()
{
   gl_FragColor = vec4(1);
}
