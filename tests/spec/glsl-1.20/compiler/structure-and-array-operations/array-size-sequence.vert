/* [config]
 * expect_result: pass
 * glsl_version: 1.20
 * [end config]
 *
 * From page 19 (page 25 of the PDF) of the GLSL 1.20 spec:
 *
 *     "When an array size is specified in a declaration, it must be an
 *     integral constant expression (see Section 4.3.3 "Constant Expressions")
 *     greater than zero."
 *
 * Section 4.3.3 says that a constant expression is "an expression formed by
 * an operator on operands that are all constant expressions."  Section 5.9
 * says "The sequence (,) operator that operates on expressions by returning
 * the type and value of the rightmost expression in a comma separated list
 * of expressions."
 */
#version 120

uniform float a[5,3]; // array of 3 float

void main() { gl_Position = vec4(0.0); }
