// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// Section 4.3.3 (Constant Expressions) of the GLSL 4.50 Spec says:
//
// "A constant expression is one of:
//  an expression formed by an operator on operands that are all constant
//  expressions, including getting an element of a constant array, or a member
//  of a constant structure, or components of a constant vector.
//  However, the lowest precedence operators of the sequence operator (,) and
//  the assignment operators ( =, +=,...) are not included in the operators
//  that can create a constant expression.

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

const int start[3] = int[3](3, 2, 1);

layout(location = start[2] + 2) in vec4 b;

void main()
{
	gl_Position = b;
}
