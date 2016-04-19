## coding=utf-8
<%inherit file="shader.vert.mako"/>\
<%block name="global_variables">\
uniform ${mat} expected;

in ${mat} value;
</%block>
<%block name="main">\
% for idx, column in enumerate(columns):
% if column == 1:
    if (value[${idx}] != expected[${idx}]) {
        fs_color = RED;
	return;
    }
% endif
% endfor
</%block>
