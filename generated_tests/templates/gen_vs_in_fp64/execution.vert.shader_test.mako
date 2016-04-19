## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
[require]
GLSL >= ${glsl_version}
% if ver == 'GL_ARB_vertex_attrib_64bit':
GL_ARB_gpu_shader_fp64
${ver}
% endif
<%block name="require"/>\

[vertex shader]
<%block name="vertex_shader"/>\

[fragment shader]
#version 150

in vec4 fs_color;
out vec4 color;

void main()
{
    color = fs_color;
}

[vertex data]
<%block name="vertex_data"/>\

[test]\
<%block name="test_commands"/>\
