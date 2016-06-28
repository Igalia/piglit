// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// If a function name is declared twice with the same parameter types,
// then the return types _and all qualifiers_ must match.

#version 130
#extension GL_MESA_shader_integer_functions : enable

void foo(int x) {}
void foo(const int x) {}	/* `const` is mismatched. */
