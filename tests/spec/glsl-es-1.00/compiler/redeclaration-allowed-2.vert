// [config]
// expect_result: pass
// glsl_version: 1.00
// [end config]

// See Section 4.2.7 "Redeclarations and Redefinitions Within the Same
// Scope"

#version 100

precision mediump float;

int f(int a);

int f(int a)
{
	return a;
}

void main()
{
	gl_Position = vec4(f(1));
}
