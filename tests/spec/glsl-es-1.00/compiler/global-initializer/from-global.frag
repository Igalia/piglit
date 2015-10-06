#version 100

/* [config]
 * expect_result: fail
 * glsl_version: 1.00
 * [end config]
 *
 * Section 4.3 (Storage Qualifiers) of the OpenGL ES 1.00.17 spec says:
 *
 *     "Declarations of globals without a storage qualifier, or with just the
 *     const qualifier, may include initializers, in which case they will be
 *     initialized before the first line of main() is executed.  Such
 *     initializers must be a constant expression."
 *
 * This differs from desktop GLSL.  A compiler that only has to support GLSL
 * ES could possibly be some amount smaller due to this restriction.
 */

precision mediump float;

const float cf = 2.0;
float gf1 = 1.0;
float gf2 = gf1;

void main()
{
    gl_FragData[0] = vec4(gf1, gf2, cf, 1.0);
}
