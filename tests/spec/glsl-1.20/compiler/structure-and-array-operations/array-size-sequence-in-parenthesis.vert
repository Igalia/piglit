/* [config]
 * expect_result: pass
 * glsl_version: 1.20
 * [end config]
 *
 * The body of the GLSL spec is slient on this issue, but the grammar says:
 *
 *     expression:
 *         assignment_expression
 *         expression COMMA assignment_expression
 *
 *     constant_expression:
 *         conditional_expression
 *
 *     ...
 *
 *     init_declarator_list:
 *         single_declaration
 *         init_declarator_list COMMA IDENTIFIER
 *         init_declarator_list COMMA IDENTIFIER LEFT_BRACKET RIGHT_BRACKET
 *         init_declarator_list COMMA IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET
 *         init_declarator_list COMMA IDENTIFIER LEFT_BRACKET RIGHT_BRACKET EQUAL initializer
 *         init_declarator_list COMMA IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET EQUAL initializer
 *         init_declarator_list COMMA IDENTIFIER EQUAL initializer
 *
 * This also matches C and C++.
 */
#version 120

uniform float a[(3,5)];

void main() { gl_Position = vec4(a[4]); }
