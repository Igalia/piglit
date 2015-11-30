// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//    "If the declared input is a structure or block, its members will be
//    assigned consecutive locations in their order of declaration, with the
//    first member assigned the location provided in the layout qualifier. For
//    a structure, this process applies to the entire structure. It is a
//    compile-time error to use a location qualifier on a member of a
//    structure.  For a block, this process applies to the entire block, or
//    until the first member is reached that has a location layout qualifier.
//    When a block member is declared with a location qualifier, its location
//    comes from that qualifier: The member's location qualifier overrides the
//    block-level declaration."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0) in block {
	float f1;
	layout(location = 1) float f2;
};

float foo(void) {
	return f1 + f2;
}
