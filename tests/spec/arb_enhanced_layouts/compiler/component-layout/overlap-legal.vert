// [config]
// expect_result: pass
// glsl_version: 1.40
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
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
//
//   ...
//
//   "The one exception where component aliasing is permitted is for two input
//   variables (not block members) to a vertex shader, which are allowed to
//   have component aliasing. This vertex-variable component aliasing is
//   intended only to support vertex shaders where each execution path
//   accesses at most one input per each aliased component.  Implementations
//   are permitted, but not required, to generate link-time errors if they
//   detect that every path through the vertex shader executable accesses
//   multiple inputs aliased to any single component."

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require

uniform int i;

// consume Y/Z/W components
layout(location = 0, component = 1) in vec3 a;

// consumes W component
layout(location = 0, component = 3) in float b;

void main()
{
  if (i == 1)
    gl_Position = vec4(b);
  else
    gl_Position = vec4(a, 1.0);
}
