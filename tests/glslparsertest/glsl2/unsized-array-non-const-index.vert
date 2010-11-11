// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

uniform int n;
void main()
{
	for (int i = 0; i < n; i++) {
		gl_TexCoord[i] = vec4(0.5, 0.5, 0.5, 0.5) * float(i);
	}
}
