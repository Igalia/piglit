<%! from six.moves import range %>
## Test execution of unpack2x16 functions in the vertex shader.
[require]
${func.requirements}

[vertex shader]
#ifndef GL_ES
#extension GL_ARB_shading_language_packing : require
#endif

const vec4 red = vec4(1, 0, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

uniform highp uint func_input;

uniform bool exact;

% for j in range(func.num_valid_outputs):
uniform ${func.result_precision} ${func.vector_type} expect${j};
% endfor

in vec4 vertex;
out vec4 vert_color;

void main()
{
    gl_Position = vertex;

    ${func.result_precision} ${func.vector_type} actual = ${func.name}(func_input);

    if (false
        % for i in range(func.num_valid_outputs):
        || (exact ? actual == expect${i} : distance(actual, expect${i}) < 0.00001)
        % endfor
       ) {
        vert_color = green;
    } else {
        vert_color = red;
    }
}

[fragment shader]
#ifdef GL_ES
precision highp float;
#endif

in vec4 vert_color;
out vec4 frag_color;

void main()
{
    frag_color = vert_color;
}

[vertex data]
vertex/float/2
-1.0 -1.0
 1.0 -1.0
 1.0  1.0
-1.0  1.0

[test]
% for io in func.inout_seq:
uniform uint func_input ${io.input}
% if func.exact:
uniform int exact 1
% else:
uniform int exact ${int(int(io.input[:-1]) in (0x0, 0xffffffff, 0x80808080,
                                               0x81818181))}
% endif
% for j in range(func.num_valid_outputs):
uniform ${func.vector_type} expect${j} ${" ".join(io.valid_outputs[j])}
% endfor
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0

% endfor
