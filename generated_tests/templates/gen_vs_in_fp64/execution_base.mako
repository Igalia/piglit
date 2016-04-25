## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()

    def rows(in_type):
        if 'vec' in in_type or 'mat' in in_type:
            return int(in_type[-1:])
        else:
            return 1

    def cols(in_type):
        if 'mat' in in_type:
            if 'x' in in_type:
                return int(in_type[-3:][:1])
            else:
                return int(in_type[-1:])
        else:
            return 1
%>\
<%! from six.moves import range %>\
[require]
GLSL >= ${glsl_version}
% if ver == 'GL_ARB_vertex_attrib_64bit':
GL_ARB_gpu_shader_fp64
${ver}
% endif
## GL_MAX_VERTEX_ATTRIBS >= ${num_vs_in}
${next.body()}\
[vertex data]
% for idx, in_type in enumerate(in_types):
% if idx == position_order - 1:
piglit_vertex/vec3/3 \
% endif
% for i in range(arrays[idx]):
% for j in range(cols(in_type)):
value${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}/${in_type}/${rows(in_type)}${'/{}'.format(j) if cols(in_type) > 1 else ''} \
% endfor
% endfor
% endfor
% if position_order > len(in_types):
piglit_vertex/vec3/3\
% endif

% for d in range(len(dvalues)):
% for vertex in ('-1.0 -1.0  0.0', ' 1.0 -1.0  0.0', ' 1.0  1.0  0.0', '-1.0  1.0  0.0'):
% for idx, in_type in enumerate(in_types):
% if idx == position_order - 1:
${vertex}   \
% endif
% for i in range(arrays[idx]):
% for j in range(cols(in_type)):
% for k in range(rows(in_type)):
${dvalues[(d + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + k) % 4]}  \
% endfor
 \
% endfor
% endfor
% endfor
% if position_order > len(in_types):
${vertex}\
% endif

% endfor
% endfor

[test]
clear color 0.0 0.0 1.0 0.0

% for d in range(len(dvalues)):
% for idx, in_type in enumerate(in_types):
% for i in range(arrays[idx]):
uniform ${in_type} expected${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}\
% for j in range(cols(in_type)):
% for k in range(rows(in_type)):
 ${dvalues[(d + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + k) % 4]}\
% endfor
% endfor

% endfor
% endfor
clear
draw arrays GL_TRIANGLE_FAN ${d * 4} 4
probe all rgba 0.0 1.0 0.0 1.0

% endfor
