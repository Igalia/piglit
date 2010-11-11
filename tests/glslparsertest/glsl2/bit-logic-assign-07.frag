// [config]
// expect_result: fail
// glsl_version: 1.30
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

// Expected: FAIL, glsl == 1.30
//
// Description: bit-and assignment with argument type (ivec2, ivec3)
//
// From page 50 (page 56 of PDF) of the GLSL 1.30 spec:
// "The operands cannot be vectors of differing size."

#version 130
void main() {
    ivec2 v = ivec2(1, 2);
    v &= ivec3(1, 2, 3);
}
