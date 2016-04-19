## coding=utf-8
<%inherit file="execution.vert.shader_test.mako"/>\
<%! from six.moves import range %>\
<%block name="require">\
GL_MAX_VERTEX_ATTRIBS >= ${num_vs_in}
</%block>
<%block name="vertex_data">\
% for idx, in_type in enumerate(in_types):
% if idx == position_order - 1:
piglit_vertex/vec3/3 \
% endif
% for i in range(arrays[idx]):
% for j in range(self.cols(in_type)):
value${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}/${in_type}/${self.rows(in_type)}${'/{}'.format(j) if self.cols(in_type) > 1 else ''} \
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
% for j in range(self.cols(in_type)):
% for k in range(self.rows(in_type)):
${dvalues[(d + (i * self.cols(in_type) + j) * self.rows(in_type) + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + (i * self.cols(in_type) + j) * self.rows(in_type) + k) % len(hvalues)]}  \
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
</%block>
<%block name="test_commands">\
% for d in range(len(dvalues)):

% for idx, in_type in enumerate(in_types):
% for i in range(arrays[idx]):
uniform ${in_type} expected${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}\
% for j in range(self.cols(in_type)):
% for k in range(self.rows(in_type)):
 ${dvalues[(d + (i * self.cols(in_type) + j) * self.rows(in_type) + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + (i * self.cols(in_type) + j) * self.rows(in_type) + k) % len(hvalues)]}\
% endfor
% endfor

% endfor
% endfor
clear color 0.0 0.0 1.0 0.0
clear
draw arrays GL_TRIANGLE_FAN ${d * 4} 4
probe all rgba 0.0 1.0 0.0 1.0
% endfor
</%block>
<%block name="vertex_shader">\
<%include file="regular_shader.vert.mako"/>\
</%block>

