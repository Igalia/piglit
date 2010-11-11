// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

varying float f;
void main()
{
    f = 1.0;  // varyings cannot be written to in a fragment shader, they can be written to in a vertex shader
}
