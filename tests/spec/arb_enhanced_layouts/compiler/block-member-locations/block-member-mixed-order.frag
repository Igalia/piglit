// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//    "The values used for locations do not have to be declared in increasing
//    order."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0) in block {
	vec4 a;
	layout(location = 2) float f1;
	float f2;
	layout(location = 1) float f3;
};

float foo(void) {
	return f1 + f2 + f3 + a.x + a.y + a.z + a.w;
}
