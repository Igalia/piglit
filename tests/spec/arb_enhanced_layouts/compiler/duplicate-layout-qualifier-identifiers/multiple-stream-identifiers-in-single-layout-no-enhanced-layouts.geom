// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader5
// check_link: false
// [end config]
//
// From the ARB_enhanced_layouts spec:
//
//   "More than one layout qualifier may appear in a single declaration.
//   Additionally, the same layout-qualifier-name can occur multiple times
//   within a layout qualifier or across multiple layout qualifiers in the
//   same declaration"

#version 150
#extension GL_ARB_gpu_shader5 : enable

layout(points) in;
layout(points) out;

layout(stream=2, stream=1) out vec3 var;

void main()
{
}
