## coding=utf-8
<%inherit file="execution.vert.shader_test.mako"/>\
<%! from six.moves import range %>\
<%block name="vertex_data">\
piglit_vertex/vec3/3\
% for i in range(self.cols(mat)):
 value/${mat}/${self.rows(mat)}${'/{}'.format(i) if self.cols(mat) > 1 else ''}\
% endfor

% for d in range(len(dvalues)):
% for vertex in ('-1.0 -1.0  0.0', ' 1.0 -1.0  0.0', ' 1.0  1.0  0.0', '-1.0  1.0  0.0'):
${vertex} \
% for i in range(self.cols(mat)):
 \
% for j in range(self.rows(mat)):
  ${dvalues[(d + i * self.rows(mat) + j) % len(dvalues)]}\
% endfor
% endfor

% endfor
% endfor
</%block>
<%block name="test_commands">\
% for d in range(len(dvalues)):

uniform ${mat} expected\
% for i in range(self.cols(mat)):
% for j in range(self.rows(mat)):
 ${dvalues[(d + i * self.rows(mat) + j) % len(dvalues)]}\
% endfor
% endfor

clear color 0.0 0.0 1.0 0.0
clear
draw arrays GL_TRIANGLE_FAN ${d * 4} 4
probe all rgba 0.0 1.0 0.0 1.0
% endfor
</%block>
<%block name="vertex_shader">\
<%include file="columns_shader.vert.mako"/>\
</%block>