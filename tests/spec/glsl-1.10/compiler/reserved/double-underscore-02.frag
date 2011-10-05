// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]
//
// Check that variable names that contain a double underscore, and the double
// underscore is located in the middle of the variable name, are reserved,
//
// From page 14 (20 of pdf) of the GLSL 1.10 spec:
//     "In addition, all identifiers containing two consecutive underscores
//     (__) are reserved as possible future keywords."

int f()
{
	int i_am_reserved__;
	return 0;
}
