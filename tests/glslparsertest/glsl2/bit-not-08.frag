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
// Description: bit-not with uvec2 argument
//
// From page 50 (page 56 of PDF) of the GLSL 1.30 spec:
// "The operand must be of type signed or unsigned integer or integer vector,"

#version 130
void main() {
    vec2 v = ~ vec2(1, 2);
}
