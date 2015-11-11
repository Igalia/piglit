// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require

// consume X/Y/Z components
layout(location = 0) in vec3 a;

// consumes W component
layout(location = 0, component = 3) in float b;

void main()
{
  gl_Position = vec4(a, b);
}
