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
 */

precision mediump float;

in float af;
float gf1 = 1.0;
float gf2 = af;
out vec4 fragdata;

void main()
{
    fragdata = vec4(gf1, gf2, af, 1.0);
}
