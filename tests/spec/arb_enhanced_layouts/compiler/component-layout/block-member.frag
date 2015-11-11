// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "Of these, variables and block members (but not blocks) additionally
//   allow the component layout qualifier."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_separate_shader_objects: require

in block {
	layout(location = 1, component = 3) float f;
};

float foo(void) {
	return f;
}
