// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: true
// require_extensions: GL_AMD_vertex_shader_layer
// [end config]
//
// In this test we verify that gl_Layer can be part of gl_PerVertex
// redeclaration when GL_AMD_vertex_shader_layer is enabled.

#version 150
#extension GL_AMD_vertex_shader_layer : enable

out gl_PerVertex {
    vec4 gl_Position;
    int gl_Layer;
};


void main()
{
}
