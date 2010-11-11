// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL: assignment of a vec2 to a float */
void main()
{
	float a;
	vec4 b;

	b.x = 6.0;
	a = b.xy;
}
