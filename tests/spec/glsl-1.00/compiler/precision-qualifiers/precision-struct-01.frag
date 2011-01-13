// [config]
// expect_result: fail
// glsl_version: 1.00
// [end config]
//
// Precision qualifiers cannot be applied to structs.
//
// This test declares the struct and its sole member with matching
// precision qualifier.
//
// See section 4.5.2 of the GLSL 1.00 spec.

#version 100

highp struct s {
	highp float a;
};

float f() {
	return 0.0;
}
