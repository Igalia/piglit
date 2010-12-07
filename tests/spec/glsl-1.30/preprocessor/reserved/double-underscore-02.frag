// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that macro names end with a doule underscore are reserved.
//
// From page 11 (17 of pdf) of the GLSL 1.30 spec:
//     "All macro names containing two consecutive underscores ( __ ) are
//     reserved for future use as predefined macro names."

#version 130
#define I_AM_RESERVED__ 1

int f()
{
	return 0;
}
