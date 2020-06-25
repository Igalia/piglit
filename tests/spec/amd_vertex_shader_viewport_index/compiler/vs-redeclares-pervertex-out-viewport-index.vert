// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: true
// require_extensions: GL_AMD_vertex_shader_viewport_index
// [end config]
//
// In this test we verify that gl_ViewportIndex can be part of gl_PerVertex
// redeclaration when GL_AMD_vertex_shader_viewport_index is enabled.
// See https://gitlab.freedesktop.org/mesa/mesa/-/issues/2946

#version 150
#extension GL_AMD_vertex_shader_viewport_index : enable

out gl_PerVertex {
    vec4 gl_Position;
    int gl_ViewportIndex;
};


void main()
{
}
