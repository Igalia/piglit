// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_NV_viewport_array2
// check_link: false
// [end config]

#version 150
#extension GL_NV_viewport_array2: require

layout (viewport_relative) out int layer;

void main()
{
  layer = 1;
}
