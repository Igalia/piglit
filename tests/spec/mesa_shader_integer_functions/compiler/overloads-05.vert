// [config]
// expect_result: pass
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// Test overload resolution where all candidates require implicit
// conversions. Under unextended GLSL 1.30, resolution is ambiguous,
// since both functions require implicit conversions.

#version 130
#extension GL_MESA_shader_integer_functions : enable

void foo(float x, int y, float z) {}
void foo(float x, int y, int z) {}	/* better for `z` */

void bar()
{
	int a = 0;
	int b = 1;
	int c = 2;

	foo(a, b, c);
}
