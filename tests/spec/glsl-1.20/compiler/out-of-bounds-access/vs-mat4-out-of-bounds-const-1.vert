// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/* From section 5.6 "Matrix Components" of the GLSL 1.20 spec:
 *
 *  "It is an error to access a matrix with a constant expression
 *   that is outside the bounds of thematrix."
 */

#version 120

void main()
{
    mat4 mat = mat4(1.0);
    mat[-1];
}
