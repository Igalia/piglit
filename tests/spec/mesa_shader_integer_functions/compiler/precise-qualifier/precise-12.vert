// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that a precise redeclaration of a variable from an enclosing scope is not
// allowed. this seems unreasonable.

// if we were to support this, it seems there would be two options for the semantics
// to impose:
//	- have `precise` only apply to uses of x within the function.
//	- have x globally marked precise from this point on, and apply the usual
//	  no-redeclaration-after-first-use rules, in program order.

#version 130
#extension GL_MESA_shader_integer_functions: require

float x;

void foo() {
	precise x;	/* seems unreasonable */
}
