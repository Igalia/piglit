## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
#version ${glsl_version_int}
% for extension in extensions:
#extension ${extension} : require
% endfor

${next.body()}\
