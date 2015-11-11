// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require

// consumes W component of 32 vectors
layout(location = 0, component = 3) in float a[32];

// consume X/Y/Z components of 32 vectors
layout(location = 0) in vec3 b[32];

void main()
{
  gl_Position = vec4(a[0], b[31]);
}
