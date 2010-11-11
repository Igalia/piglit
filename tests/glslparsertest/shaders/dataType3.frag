// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 110

/*
 * In GLSL 1.10 uniform initializers are illegal
 * In GLSL 1.20 or later, uniform initializers are allowed
 */

uniform int i = 1; // uniforms are read only in GLSL 1.10

void main()
{
}
