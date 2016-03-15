## coding=utf-8
<%inherit file="compiler.mako"/>\
<%
    var_name = 'u{}'.format('[4]' if array else '')
    var_as_vec4 = '{}u{}{}'.format('s.' if struct else '',
                                   '[3]' if array else '',
                                   '[0]' if type_name.startswith('dmat') else '')
    if type_name.endswith('2'):
        var_as_vec4 += '.xyxy'
    elif type_name.endswith('3'):
        var_as_vec4 += '.xyzx'
%>\
<%block name="comments">\
 * Declare a ${mode} interpolation ${type_name}\
% if array:
 array\
% endif
% if struct:
 inside a struct\
% endif
% if interface_block:
 in an interface block\
% endif
.
</%block>\
% if struct:
struct S {
	${type_name} ${var_name};
};

% endif
% if interface_block:
in IB {
	\
% endif
% if mode != 'default':
${mode} \
% endif
% if not interface_block:
in \
% endif
% if struct:
S s;
% else:
${type_name} ${var_name};
% endif
% if interface_block:
};
% endif
out vec4 color;

void main()
{
	color = vec4(${var_as_vec4});
}
