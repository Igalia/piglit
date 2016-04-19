## coding=utf-8
<%inherit file="shader_base.mako"/>\
% for idx, in_type in enumerate(in_types):
uniform ${in_type} expected${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor

% for idx, in_type in enumerate(in_types):
in ${in_type} value${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor

in vec3 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
	gl_Position = vec4(piglit_vertex, 1.0);
% for idx, in_type in enumerate(in_types):
	if (value${idx} != expected${idx}) {
		fs_color = RED;
		return;
	}
% endfor
	fs_color = GREEN;
}
