// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that variable names that end with  a double underscore are reserved.
//
// From page 16 (22 of pdf) of the GLSL 1.30 spec:
//     "In addition, all identifiers containing two consecutive underscores
//     (__) are reserved as possible future keywords."

#version 130

int f()
{
	int i_am_reserved__;
	return 0;
}
