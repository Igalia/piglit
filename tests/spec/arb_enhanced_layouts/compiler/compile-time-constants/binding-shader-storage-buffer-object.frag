// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_storage_buffer_object GL_ARB_enhanced_layouts
// [end config]

#version 150
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout (binding = start + 4) buffer buf {
	float f;
};

float foo(void) {
	return f;
}
