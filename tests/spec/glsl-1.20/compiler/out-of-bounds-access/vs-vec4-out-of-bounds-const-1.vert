// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/* From section 5.5 "Vector Components" of the GLSL 1.20 spec:
 *
 *  "Reading from or writing to a vector using a constant integral expression
 *   with a value that is negative or greater than or equal to the size of
 *   the vector is illegal"
 */

#version 120

void main()
{
    vec4 vec = vec4(1.0);
    vec[-1];
}
