## Test execution of unpack2x16 functions in the fragment shader.
[require]
${func.requirements}

[vertex shader]
in vec4 vertex;

void main()
{
    gl_Position = vertex;
}

[fragment shader]
#ifndef GL_ES
#extension GL_ARB_shading_language_packing : require
#else
precision highp float;
#endif

const vec4 red = vec4(1, 0, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

uniform highp uint func_input;

uniform bool exact;

% for i in xrange(func.num_valid_outputs):
uniform ${func.result_precision} ${func.vector_type} expect${i};
% endfor

out vec4 frag_color;

void main()
{
    ${func.result_precision} ${func.vector_type} actual = ${func.name}(func_input);

    if (false
        % for i in xrange(func.num_valid_outputs):
        || (exact ? actual == expect${i}
                  : distance(actual, expect${i}) < 0.00001)
        % endfor
       ) {
        frag_color = green;
    } else {
        frag_color = red;
    }
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
% for i in xrange(func.num_valid_outputs):
uniform ${func.vector_type} expect${i} ${" ".join(io.valid_outputs[i])}
% endfor
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0

% endfor
