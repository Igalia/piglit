// [config]
// expect_result: fail
// glsl_version: 1.40
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "Location aliasing is causing two variables or block members to have the
//   same location number. Component aliasing is assigning the same (or
//   overlapping) component numbers for two location aliases. (Recall if
//   component is not used, components are assigned starting with 0.) With one
//   exception, location aliasing is allowed only if it does not cause
//   component aliasing; it is a compile-time or link-time error to cause
//   component aliasing."

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

// consume Y/Z/W components
layout(location = 0, component = 1) out vec3 a;

// consumes W component
layout(location = 0, component = 3) out float b;

void main()
{
  a = vec3(1.0);
  b = 0.0;
}
