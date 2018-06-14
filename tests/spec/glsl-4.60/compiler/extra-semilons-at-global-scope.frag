// [config]
// expect_result: pass
// glsl_version: 1.10
// [end config]
// Although the fix is in GLSL 4.60 we should allows this across all GLSL versions.

#version 110

// From the GLSL 4.60 spec, section 1.2.1 (Summary of Changes from Revision 7
// of GLSL Version 4.50):
//
//  "Private Bug 16070: Allow extra semi-colons at global scope"

uniform int i;;

void main()
{
}
