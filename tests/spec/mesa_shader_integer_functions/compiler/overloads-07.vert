// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// Test overload resolution where all candidates require implicit
// conversions. Under unextended GLSL 1.30, resolution is ambiguous,
// since both functions require implicit conversions. With MESA_shader_integer_functions,
// this case is still ambiguous, since int->float conversion is not
// considered better or worse than int->uint conversion.

#version 130
#extension GL_MESA_shader_integer_functions : enable

void foo(float x) {}
void foo(uint x) {}

void bar()
{
	int x = 0;
	foo(x);
}

