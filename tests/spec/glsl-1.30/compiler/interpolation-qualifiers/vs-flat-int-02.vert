// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Declare a non-flat integral vertex output.
//
// From section 4.3.6 of the GLSL 1.30 spec:
//     "If a vertex output is a signed or unsigned integer or integer vector,
//     then it must be qualified with the interpolation qualifier flat."

#version 130

out int x;

float f() {
	return 0.0;
}
