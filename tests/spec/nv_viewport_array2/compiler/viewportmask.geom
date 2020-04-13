// [config]
// expect_result: pass
// glsl_version: 1.10
// require_extensions: GL_NV_viewport_array2
// check_link: false
// [end config]

#version 110
#extension GL_NV_viewport_array2: require

void main()
{
  gl_ViewportMask[0] = 1;
}
