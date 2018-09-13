<%!
import six
import numpy as np
%>
[require]
GLSL >= ${version}
% for extension in extensions:
${extension}
% endfor

% if execution_stage == 'vs':
[vertex shader]
#version ${int(float(version) * 100)}
% for extension in extensions:
#extension ${extension}: require
% endfor

#define SUM_N_to_1(x)  (((uint(x)+1u)*uint(x))/2u)

% if output_type in {"uint", "int"}:
/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.  ${func} is commutative, so instead of explicitly
 * storing N^2 results, we store N+(N-1)+(N-2)+...+1 = (N+1)*N/2 results.  The
 * total storage requirement is (N+1)*N/2+N.  N=43 would require 989
 * components, and N=44 would require 1034 components.
 *
 * The storage requirement can be further reduced by observing that the diagonal
 * of the result matrix is always 0 because ${func}(x, x) == 0.  The new total
 * storage requirement is (((N+1)*N/2)-N)+N (because the diagonal is N
 * elements).  N=44 would require 990 components, and N=45 would require 1035
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform ${input_type} src[44];
uniform ${output_type} expected[int(SUM_N_to_1(src.length())) - src.length()];
% else:
/* OpenGL 4.0 requires that implementations support uniform buffer blocks of at
 * least 16384 bytes, and each value is 8 bytes.  This results in a maximum of
 * 2048 components.  ${func} is commutative, so instead of explicitly storing
 * N^2 results, we store N+(N-1)+(N-2)+...+1 = (N+1)*N/2 results.  N=63 would
 * require 2016 components, and N=64 would require 2080 components.
 *
 * The storage requirement can be further reduced by observing that the
 * diagonal of the result matrix is always 0 because ${func}(x, x) == 0.  The
 * new total storage requirement is ((N+1)*N/2)-N (because the diagonal is N
 * elements).  N=64 would require 2016 components, and N=65 would require
 * 2080 components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform Sources { ${input_type} src[64]; };
uniform Expected { u64vec2 expected[(int(SUM_N_to_1(src.length())) - src.length() + 1) / 2]; };
% endif

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

${output_type} get_expected_result(uint i, uint j)
{
    if (i == j)
        return ${output_type}(0);

    uint row = min(i, j);
    uint col = max(i, j) - row;

    uint k = uint(src.length() - 1) - row;
    uint idx = SUM_N_to_1(src.length() - 1) - SUM_N_to_1(k) + (col - 1u);

    % if output_type in {"uint", "int"}:
    return expected[idx];
    % else:
    u64vec2 tmp = expected[idx / 2u];
    return ${output_type}((idx & 1u) == 0u ? tmp.x : tmp.y);
    % endif
}

void main()
{
    gl_Position = piglit_vertex;

    color = vec4(0.0, 1.0, 0.0, 1.0);

    uvec4 r = ranges[2u * uint(piglit_vertex.x > 0.0) +
                     uint(piglit_vertex.y > 0.0)];

    for (uint i = r.x; i < r.y; i++) {
        for (uint j = r.z; j < r.w; j++) {
            if (${func}(src[i], src[j]) != get_expected_result(i, j))
                color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    }

}
% else:
[vertex shader passthrough]
% endif

[fragment shader]
% if execution_stage == 'fs':
#version ${int(float(version) * 100)}
% for extension in extensions:
#extension ${extension}: require
% endfor

#define SUM_N_to_1(x)  (((uint(x)+1u)*uint(x))/2u)

% if output_type in {"uint", "int"}:
/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.  ${func} is commutative, so instead of explicitly
 * storing N^2 results, we store N+(N-1)+(N-2)+...+1 = (N+1)*N/2 results.  The
 * total storage requirement is (N+1)*N/2+N.  N=43 would require 989
 * components, and N=44 would require 1034 components.
 *
 * The storage requirement can be further reduced by observing that the diagonal
 * of the result matrix is always 0 because ${func}(x, x) == 0.  The new total
 * storage requirement is (((N+1)*N/2)-N)+N (because the diagonal is N
 * elements).  N=44 would require 990 components, and N=45 would require 1035
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform ${input_type} src[44];
uniform ${output_type} expected[int(SUM_N_to_1(src.length())) - src.length()];
% else:
/* OpenGL 3.1 requires that implementations support uniform buffer blocks of at
 * least 16384 bytes, and each value is 8 bytes.  This results in a maximum of
 * 2048 components.  ${func} is commutative, so instead of explicitly storing
 * N^2 results, we store N+(N-1)+(N-2)+...+1 = (N+1)*N/2 results.  N=63 would
 * require 2016 components, and N=64 would require 2080 components.
 *
 * The storage requirement can be further reduced by observing that the
 * diagonal of the result matrix is always 0 because ${func}(x, x) == 0.  The
 * new total storage requirement is ((N+1)*N/2)-N (because the diagonal is N
 * elements).  N=64 would require 2016 components, and N=65 would require
 * 2080 components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform Sources { ${input_type} src[64]; };
uniform Expected { u64vec2 expected[(int(SUM_N_to_1(src.length())) - src.length() + 1) / 2]; };
% endif

${output_type} get_expected_result(uint i, uint j)
{
    if (i == j)
        return ${output_type}(0);

    uint row = min(i, j);
    uint col = max(i, j) - row;

    uint k = uint(src.length() - 1) - row;
    uint idx = SUM_N_to_1(src.length() - 1) - SUM_N_to_1(k) + (col - 1u);

    % if output_type in {"uint", "int"}:
    return expected[idx];
    % else:
    u64vec2 tmp = expected[idx / 2u];
    return ${output_type}((idx & 1u) == 0u ? tmp.x : tmp.y);
    % endif
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

    if (${func}(src[i], src[j]) == get_expected_result(i, j))
        piglit_fragcolor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        piglit_fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
    % else:
    piglit_fragcolor = color;
    % endif
}

[test]
% if output_type in {"uint", "int"}:
    % for i, s in enumerate(sources):
uniform ${input_type} src[${i}] ${"{:#011x}".format(np.uint32(s))}
    % endfor

    % for i, s in enumerate(results):
uniform ${output_type} expected[${i}] ${"{:#010x}".format(np.uint32(s))}
    % endfor
% else:
    % for i, s in enumerate(sources):
uniform ${input_type} src[${i}] ${"{:#018x}".format(np.uint64(s))}
    % endfor

    % for i in range((len(results) + 1) // 2):
uniform u64vec2 expected[${i}] ${"{:#018x}".format(np.uint64(results[i*2 + 0]))} ${"{:#018x}".format(np.uint64(results[i*2 + 1])) if len(results) > (i*2 + 1) else "0xDEADBEEFDEADBEEF"}
    % endfor
%endif

draw rect -1 -1 2 2
probe all rgb 0.0 1.0 0.0
