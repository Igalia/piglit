## Test execution of pack2x16 functions in the vertex shader.
[require]
${func.requirements}

[vertex shader]
#ifndef GL_ES
#extension GL_ARB_shading_language_packing : require
#endif

const vec4 red = vec4(1, 0, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

uniform ${func.vector_type} func_input;

% for j in range(func.num_valid_outputs):
uniform ${func.result_precision} uint expect${j};
% endfor

in vec4 vertex;
out vec4 vert_color;

void main()
{
    gl_Position = vertex;
    ${func.result_precision} uint actual = ${func.name}(func_input);

    if (false
        % for j in range(func.num_valid_outputs):
        || actual == expect${j}
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
uniform ${func.vector_type} func_input ${" ".join(io.input)}
% for j in range(func.num_valid_outputs):
uniform uint expect${j} ${io.valid_outputs[j]}
% endfor
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0

% endfor
