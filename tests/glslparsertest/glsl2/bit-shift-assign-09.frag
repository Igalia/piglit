// [config]
// expect_result: fail
// glsl_version: 1.30
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

// Expected: FAIL, glsl <= 1.30
//
// Description: bit-shift-assign with arguments (mat4, int)
//
// From page 50 (page 56 of the PDF) of the GLSL 1.30 spec:
// "the operands must be signed or unsigned integers or integer vectors."

#version 130
void main() {
    bool b = true;
    b <<= b;
}
