// [config]
// expect_result: fail
// glsl_version: 1.00
// [end config]

// See Section 4.2.7 "Redeclarations and Redefinitions Within the Same
// Scope"

#version 100

precision mediump float;

int f(int a);
struct f { int x; };

void main()
{
	gl_Position = vec4(1.0);
}
