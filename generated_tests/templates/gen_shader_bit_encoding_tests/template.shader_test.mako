[require]
GLSL >= ${version}
% for extension in extensions:
${extension}
% endfor

[vertex shader]
% if execution_stage == 'vs':
% for extension in extensions:
#extension ${extension}: enable
% endfor

uniform ${input_type} given;
uniform ${output_type} expected;
out vec4 color;
% endif

in vec4 vertex;

void main() {
    gl_Position = vertex;

    % if execution_stage == 'vs':
    color = vec4(0.0, 1.0, 0.0, 1.0);

    if (expected.x != ${func}(${in_modifier_func}(given.x)))
            color.r = 1.0;
    if (expected.xy != ${func}(${in_modifier_func}(given.xy)))
            color.r = 1.0;
    if (expected.xyz != ${func}(${in_modifier_func}(given.xyz)))
            color.r = 1.0;
    if (expected != ${func}(${in_modifier_func}(given)))
            color.r = 1.0;
    % endif
}

[fragment shader]
% if execution_stage == 'fs':
% for extension in extensions:
#extension ${extension}: enable
% endfor

uniform ${input_type} given;
uniform ${output_type} expected;
% else:
in vec4 color;
% endif

out vec4 frag_color;

void main() {
    % if execution_stage == 'fs':
    frag_color = vec4(0.0, 1.0, 0.0, 1.0);

    if (expected.x != ${func}(${in_modifier_func}(given.x)))
            frag_color.r = 1.0;
    if (expected.xy != ${func}(${in_modifier_func}(given.xy)))
            frag_color.r = 1.0;
    if (expected.xyz != ${func}(${in_modifier_func}(given.xyz)))
            frag_color.r = 1.0;
    if (expected != ${func}(${in_modifier_func}(given)))
            frag_color.r = 1.0;
    % else:
    frag_color = color;
    % endif
}

[vertex data]
vertex/float/2
-1.0 -1.0
 1.0 -1.0
 1.0  1.0
-1.0  1.0

[test]
% for name, data in sorted(test_data.iteritems()):
% if name == '-0.0' and in_modifier_func != '' and func == 'intBitsToFloat':
# ${in_modifier_func}(INT_MIN) doesn't fit in a 32-bit int. Cannot test.
% else:
# ${name}
uniform ${input_type} given ${' '.join(str(in_func(d)) for d in data)}
uniform ${output_type} expected ${' '.join(str(out_func(modifier_func(in_func(d)))) for d in data)}
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0
% endif

% endfor
