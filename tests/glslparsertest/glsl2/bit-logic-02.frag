// [config]
// expect_result: pass
// glsl_version: 1.30
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

// Expected: PASS, glsl == 1.30
//
// Description: bit-logic ops with argument type (uint, uint)
//
// From page 50 (page 56 of PDF) of the GLSL 1.30 spec:
// "The operands must be of type signed or unsigned integers or integer
// vectors."

#version 130
void main() {
    uint x0 = uint(0) & uint(1);
    uint x1 = uint(0) | uint(1);
    uint x2 = uint(0) ^ uint(1);
}
