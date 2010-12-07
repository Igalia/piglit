// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that the compiler raises an error when an undefined macro is used as
// an argument to the #if directive.
//
// From page 11 (17 of pdf) of the GLSL 1.30 spec:
//     "It is an error to use #if or #elif on expressions containing
//     undefined macro names, other than as arguments to the
//     defined operator."

#version 130

#if UNDEFINED_MACRO
#	define SHOULD_NOT_REACH_HERE 1
#endif

int f()
{
	return 0;
}
