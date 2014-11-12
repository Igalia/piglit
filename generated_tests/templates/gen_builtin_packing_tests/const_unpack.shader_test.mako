## Test evaluation of constant unpack2x16 expressions.
[require]
${func.requirements}

[vertex shader]
#ifndef GL_ES
#extension GL_ARB_shading_language_packing : require
#endif

const vec4 red = vec4(1, 0, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

in vec4 vertex;
out vec4 vert_color;

void main()
{
    ${func.result_precision} ${func.vector_type} actual;

    gl_Position = vertex;
    vert_color = green;

    % for io in func.inout_seq:
    actual = ${func.name}(${io.input});

    if (true
        % for v in io.valid_outputs:
        && actual != ${func.vector_type}(${', '.join(v)})
        % endfor
       ) {
        vert_color = red;
    }

    % endfor
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
draw arrays GL_TRIANGLE_FAN 0 4
probe all rgba 0.0 1.0 0.0 1.0
