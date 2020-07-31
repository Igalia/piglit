// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//    "For some blocks declared as arrays, the location can only be applied
//    at the block level: When a block is declared as an array where
//    additional locations are needed for each member for each block array
//    element, it is a compile-time error to specify locations on the block
//    members. That is, when locations would be under specified by applying
//    them on block members, they are not allowed on block members. For
//    arrayed interfaces (those generally having an extra level of
//    arrayness due to interface expansion), the outer array is stripped
//    before applying this rule"

// From Section 1.2.1 (Changes from Revision 6) of GLSL 4.50 spec:
//
//    "Private Bug 15678: Don’t allow location = on block members
//    where the block needs an array of locations"

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

in Block {
	layout(location = 1) float f;
} block[2];

float foo(void) {
	return block[0].f +
			 block[1].f;
}
