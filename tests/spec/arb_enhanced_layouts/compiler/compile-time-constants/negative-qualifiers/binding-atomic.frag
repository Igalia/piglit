// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_shader_atomic_counters GL_ARB_enhanced_layouts
// [end config]
//
// From Section 4.4.6 (Opaque-Uniform Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "If the binding is less than zero, or greater than or equal to the
//    implementation-dependent maximum supported number of units, a
//    compile-time error will occur."

#version 140
#extension GL_ARB_shader_atomic_counters: require
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout(binding = start - 4) uniform atomic_uint x;

void main()
{
}
