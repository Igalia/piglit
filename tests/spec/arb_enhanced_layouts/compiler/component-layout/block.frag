// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "It is a compile-time error to apply the component qualifier to a matrix,
//   a structure, a block, or an array containing any of these."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 1, component = 0) in block {
	float f;
};

float foo(void) {
	return f;
}
