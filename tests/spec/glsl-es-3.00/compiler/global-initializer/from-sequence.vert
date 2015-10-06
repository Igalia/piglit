#version 300 es

/* [config]
 * expect_result: fail
 * glsl_version: 3.00
 * [end config]
 *
 * Section 4.3 (Storage Qualifiers) of the OpenGL ES 3.00.4 spec says:
 *
 *     "Initializers may only be used in declarations of globals with no
 *     storage qualifier or with a const qualifier.  Such initializers must be
 *     a constant expression."
 *
 * This differs from desktop GLSL.  A compiler that only has to support GLSL
 * ES could possibly be some amount smaller due to this restriction.
 *
 * Section 4.3.3 "Constant Expressions" of the OpenGL GLSL ES 3.00.4 spec
 * says:
 *
 *     "However, the sequence operator ( , ) and the assignment operators ( =,
 *     +=, ...)  are not included in the operators that can create a constant
 *     expression."
 */

const float cf = 2.0;
float gf1 = 1.0;
float gf2 = (cf, 3.0);

void main()
{
    gl_Position = vec4(gf1, gf2, cf, 1.0);
}
