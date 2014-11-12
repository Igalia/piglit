## Test execution of pack2x16 functions in the fragment shader.
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

uniform ${func.vector_type} func_input;

% for i in xrange(func.num_valid_outputs):
uniform ${func.result_precision} uint expect${i};
% endfor

out vec4 frag_color;

void main()
{
    ${func.result_precision} uint actual = ${func.name}(func_input);

    if (false
        % for i in xrange(func.num_valid_outputs):
        || actual == expect${i}
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
uniform ${func.vector_type} func_input ${" ".join(io.input)}
% for i in xrange(func.num_valid_outputs):
uniform uint expect${i} ${io.valid_outputs[i]}
% endfor
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0

% endfor
