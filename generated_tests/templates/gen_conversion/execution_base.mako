## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
<%! from six.moves import range %>\
[require]
GLSL >= ${glsl_version}
% if ver == 'GL_ARB_gpu_shader_fp64':
${ver}
% endif
${next.body()}\
[test]
clear color 0.0 0.0 0.0 0.0

% for conversion in conversions:
clear
uniform ${uniform_from_type} from \
% for i in range(amount):
${conversion['from']} \
% endfor

uniform ${uniform_to_type} to \
% for i in range(amount):
${conversion['to']} \
% endfor

draw rect -1 -1 2 2
probe all rgba 0.0 1.0 0.0 1.0

% endfor
