// [config]
// expect_result: fail
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - built-in exp hidden by variable */
#version 120
uniform float exp;

void main()
{
    gl_Position = vec4(exp(0.0));
}
