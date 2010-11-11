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
// Description: bit-shift with unequally sized vectors
//
// See page 50 (page 56 of the PDF) of the GLSL 1.30 spec.

#version 130
void main() {
    ivec2 v = ivec2(0, 1) << ivec3(0, 1, 2);
}

