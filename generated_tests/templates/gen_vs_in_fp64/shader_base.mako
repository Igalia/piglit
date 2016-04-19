## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
#version ${glsl_version_int}
% if ver == 'GL_ARB_vertex_attrib_64bit':
#extension GL_ARB_gpu_shader_fp64 : require
#extension GL_ARB_vertex_attrib_64bit : require
% endif

${next.body()}\
