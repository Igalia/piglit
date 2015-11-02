// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// Section 4.3.3 (Constant Expressions) of the GLSL 4.50 Spec says:
//
// "A constant expression is one of:
//  valid use of the length() method on an explicitly sized object, whether or
//  not the object itself is constant (implicitly sized or run-time sized
//  arrays do not return a constant expression)"

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

int start[];

layout(location = start.length() + 2) in vec4 b;

void main()
{
	start[1] = 3;
	gl_Position = b;
}
