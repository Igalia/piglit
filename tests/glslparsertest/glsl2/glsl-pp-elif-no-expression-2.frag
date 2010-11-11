// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - since the if condition fails, the preprocessor actually encounters
 * the elif without an expression, which should then fail.
 */
void main()
{
#if 0
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
#elif
	gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
#else
	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
#endif
}
