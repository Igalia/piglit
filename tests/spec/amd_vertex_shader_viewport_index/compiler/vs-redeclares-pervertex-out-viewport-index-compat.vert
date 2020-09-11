// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// check_link: true
// require_extensions: GL_AMD_vertex_shader_viewport_index GL_AMD_vertex_shader_layer
// [end config]
//
// Using a compatibility mode + gl_Layer + gl_ViewportIndex created more builtins than
// per_vertex_accumulator::fields could store.
// Reproducer for https://gitlab.freedesktop.org/mesa/mesa/-/issues/3508

#version 150 compatibility
#extension GL_AMD_vertex_shader_viewport_index : enable
#extension GL_AMD_vertex_shader_layer : enable

out gl_PerVertex {
    vec4 gl_Position;
    int gl_ViewportIndex;
};


void main()
{
}