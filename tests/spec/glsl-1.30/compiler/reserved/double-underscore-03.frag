// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that variable names that contain a double underscore, and the double
// underscore is located in the middle of the variable name, are reserved,
//
// From page 16 (22 of pdf) of the GLSL 1.30 spec:
//     "In addition, all identifiers containing two consecutive underscores
//     (__) are reserved as possible future keywords."

#version 130

int f()
{
	int i__am__reserved;
	return 0;
}
