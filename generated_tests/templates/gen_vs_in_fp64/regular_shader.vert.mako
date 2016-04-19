## coding=utf-8
<%inherit file="shader.vert.mako"/>\
<%block name="global_variables">\
% for idx, in_type in enumerate(in_types):
uniform ${in_type} expected${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor

% for idx, in_type in enumerate(in_types):
in ${in_type} value${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor
</%block>
<%block name="main">\
% for idx, in_type in enumerate(in_types):
    if (value${idx} != expected${idx}) {
        fs_color = RED;
	return;
    }
% endfor
</%block>
