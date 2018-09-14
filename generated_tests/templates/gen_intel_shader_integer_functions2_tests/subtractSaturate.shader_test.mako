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

% if output_type in {"uint", "int"}:
/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.  subtractSaturate is not commutative, so the full set
 * of N^2 results must be stored.  The total storate requirement is (N*N)+N.
 * N=31 would require 992 components, and N=32 would require 1056 components.
 *
 * The storage requirement can be reduced by observing that the diagonal of
 * the result matrix is always 0 because subtractSaturate(x, x) == 0.  The new
 * total storage requirement is ((N-1)*N)+N.  N=32 would require 1024
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform ${input_type} src[32];
uniform ${output_type} expected[(src.length() - 1) * src.length()];
% else:
/* OpenGL 4.0 requires that implementations support uniform buffer blocks of at
 * least 16384 bytes, and each value is 8 bytes.  This results in a maximum of
 * 2048 components.  subtractSaturate is not commutative, so the full set
 * of N^2 results must be stored.  The total storate requirement is (N*N)+N.
 * N=44 would require 1980 components, and N=45 would require 2070 components.
 *
 * The storage requirement can be reduced by observing that the diagonal of
 * the result matrix is always 0 because subtractSaturate(x, x) == 0.  The new
 * total storage requirement is ((N-1)*N)+N.  N=45 would require 2025
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform Sources { ${input_type} src[45]; };
uniform Expected { u64vec2 expected[((src.length() - 1) * src.length() + 1) / 2]; };
% endif

${output_type} get_expected_result(uint i, uint j)
{
    if (i == j)
	return ${output_type}(0);

    if (i < j)
	j--;

    uint idx = (i * uint(src.length() - 1)) + j;

    % if output_type in {"uint", "int"}:
    return expected[idx];
    % else:
    u64vec2 data = expected[idx / 2u];
    return ${output_type}((idx & 1u) == 0u ? data.x : data.y);
    % endif
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
            if (subtractSaturate(src[i], src[j]) != get_expected_result(i, j))
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

% if output_type in {"uint", "int"}:
/* OpenGL 3.0 only requires that implementations support 1024 uniform
 * components per stage.  subtractSaturate is not commutative, so the full set
 * of N^2 results must be stored.  The total storate requirement is (N*N)+N.
 * N=31 would require 992 components, and N=32 would require 1056 components.
 *
 * The storage requirement can be reduced by observing that the diagonal of
 * the result matrix is always 0 because subtractSaturate(x, x) == 0.  The new
 * total storage requirement is ((N-1)*N)+N.  N=32 would require 1024
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform ${input_type} src[32];
uniform ${output_type} expected[(src.length() - 1) * src.length()];
% else:
/* OpenGL 4.0 requires that implementations support uniform buffer blocks of at
 * least 16384 bytes, and each value is 8 bytes.  This results in a maximum of
 * 2048 components.  subtractSaturate is not commutative, so the full set
 * of N^2 results must be stored.  The total storate requirement is (N*N)+N.
 * N=44 would require 1980 components, and N=45 would require 2070 components.
 *
 * The storage requirement can be reduced by observing that the diagonal of
 * the result matrix is always 0 because subtractSaturate(x, x) == 0.  The new
 * total storage requirement is ((N-1)*N)+N.  N=45 would require 2025
 * components.  One more test vector!  TOTALLY WORTH IT!
 */
uniform Sources { ${input_type} src[45]; };
uniform Expected { u64vec2 expected[((src.length() - 1) * src.length() + 1) / 2]; };
% endif

${output_type} get_expected_result(uint i, uint j)
{
    if (i == j)
	return ${output_type}(0);

    if (i < j)
	j--;

    uint idx = (i * uint(src.length() - 1)) + j;

    % if output_type in {"uint", "int"}:
    return expected[idx];
    % else:
    u64vec2 data = expected[idx / 2u];
    return ${output_type}((idx & 1u) == 0u ? data.x : data.y);
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

    if (subtractSaturate(src[i], src[j]) == get_expected_result(i, j))
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
uniform ${input_type} src[${i}] ${"{:#010x}".format(np.uint32(s))}
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
% endif

draw rect -1 -1 2 2
probe all rgb 0.0 1.0 0.0
