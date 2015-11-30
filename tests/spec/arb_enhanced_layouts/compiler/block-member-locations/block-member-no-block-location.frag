// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//    "If a block has no block-level location layout qualifier, it is required
//    that either all or none of its members have a location layout qualifier,
//    or a compile-time error results."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

in block {
	layout(location = 0) vec4 a;
	layout(location = 1) float f1;
	layout(location = 2) float f2;
	layout(location = 3) float f3;
};

float foo(void) {
	return f1 + f2 + f3 + a.x + a.y + a.z + a.w;
}
