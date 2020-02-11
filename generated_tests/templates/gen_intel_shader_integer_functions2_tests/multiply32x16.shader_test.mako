<%!
import six
import numpy as np
%>
[require]
GL >= 3.0
GLSL >= 1.30
GL_INTEL_shader_integer_functions2

% if execution_stage == 'vs':
[vertex shader]
#extension GL_INTEL_shader_integer_functions2: require

/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.
 */
uniform ${input_type} src[${len(sources)}];
uniform ${input_type} src_as_16bits[src.length()];

${output_type} get_expected_result(uint i, uint j)
{
    return src[i] * src_as_16bits[j];
}

const uvec4 ranges[] = uvec4[](uvec4(0, uint(src.length()) / 2u,
                                     0, uint(src.length()) / 2u),
                               uvec4(0, uint(src.length()) / 2u,
                                     uint(src.length()) / 2u, src.length()),
                               uvec4(uint(src.length()) / 2u, src.length(),
                                     0, uint(src.length()) / 2u),
                               uvec4(uint(src.length()) / 2u, src.length(),
                                     uint(src.length()) / 2u, src.length()));

out vec4 color;
in vec4 piglit_vertex;

void main()
{
    gl_Position = piglit_vertex;

    color = vec4(0.0, 1.0, 0.0, 1.0);

    uvec4 r = ranges[2u * uint(piglit_vertex.x > 0.0) +
                     uint(piglit_vertex.y > 0.0)];

    for (uint i = r.x; i < r.y; i++) {
        for (uint j = r.z; j < r.w; j++) {
            if (multiply32x16(src[i], src[j]) != get_expected_result(i, j))
                color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    }

}
% else:
[vertex shader passthrough]
% endif

[fragment shader]
% if execution_stage == 'fs':
#extension GL_INTEL_shader_integer_functions2: require

/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.
 */
uniform ${input_type} src[${len(sources)}];
uniform ${input_type} src_as_16bits[src.length()];

${output_type} get_expected_result(uint i, uint j)
{
    return src[i] * src_as_16bits[j];
}
% else:
in vec4 color;
% endif

out vec4 piglit_fragcolor;

void main()
{
    % if execution_stage == 'fs':
    const uint l = uint(src.length());
    uint i = uint(gl_FragCoord.x) % l;
    uint j = uint(gl_FragCoord.y) % l;

    if (multiply32x16(src[i], src[j]) == get_expected_result(i, j))
        piglit_fragcolor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        piglit_fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
    % else:
    piglit_fragcolor = color;
    % endif
}

[test]
% for i, s in enumerate(sources):
uniform ${input_type} src[${i}] ${"{:#010x}".format(np.uint32(s))}
% endfor

% if input_type == "int":
    % for i, s in enumerate(sources):
uniform ${input_type} src_as_16bits[${i}] ${"{:#010x}".format(np.uint32((np.int32(np.uint32(s) << 16) >> 16)))}
    % endfor
% else:
    % for i, s in enumerate(sources):
uniform ${input_type} src_as_16bits[${i}] ${"{:#010x}".format(np.uint32(s) & 0x0000ffff)}
    % endfor
% endif

draw rect -1 -1 2 2
probe all rgb 0.0 1.0 0.0
