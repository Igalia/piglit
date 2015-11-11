// [config]
// expect_result: fail
// glsl_version: 1.40
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader_fp64 GL_ARB_separate_shader_objects
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
#extension GL_ARB_gpu_shader_fp64: require
#extension GL_ARB_separate_shader_objects: require

// consume X/Y components
layout(location = 7, component = 0) out double a;

// consumes Y component
layout(location = 7, component = 1) out float b;

void main()
{
}
