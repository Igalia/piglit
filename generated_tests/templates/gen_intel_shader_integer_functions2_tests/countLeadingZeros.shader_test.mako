<%! import six %>
[require]
GL >= 3.0
GLSL >= 1.30
GL_INTEL_shader_integer_functions2

% if execution_stage == 'vs':
[vertex shader]
#extension GL_INTEL_shader_integer_functions2: require

uniform uint src[1024];

const uvec2 ranges[] = uvec2[](uvec2((0u * uint(src.length())) / 4u,
				     (1u * uint(src.length())) / 4u),
			       uvec2((1u * uint(src.length())) / 4u,
				     (2u * uint(src.length())) / 4u),
			       uvec2((2u * uint(src.length())) / 4u,
				     (3u * uint(src.length())) / 4u),
			       uvec2((3u * uint(src.length())) / 4u,
				     (4u * uint(src.length())) / 4u));
out vec4 color;
in vec4 piglit_vertex;

void main()
{
    gl_Position = piglit_vertex;

    color = vec4(0.0, 1.0, 0.0, 1.0);

    uvec2 r = ranges[2u * uint(piglit_vertex.x > 0.0) +
                     uint(piglit_vertex.y > 0.0)];

    for (uint i = r.x; i < r.y; i++) {
	uint expect = i % 33u;

	if (${func}(src[i]) != expect)
	    color = vec4(1.0, 0.0, 0.0, 1.0);
    }

}
% else:
[vertex shader passthrough]
% endif

[fragment shader]
% if execution_stage == 'fs':
#extension GL_INTEL_shader_integer_functions2: require

uniform uint src[1024];
% else:
in vec4 color;
% endif

out vec4 piglit_fragcolor;

void main()
{
    % if execution_stage == 'fs':
    uint i = uint(gl_FragCoord.x) % 32u;
    uint j = uint(gl_FragCoord.y) % 32u;
    uint idx = (i * 32u) + j;
    uint expect = idx % 33u;

    if (${func}(src[idx]) == expect)
        piglit_fragcolor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        piglit_fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
    % else:
    piglit_fragcolor = color;
    % endif
}

[test]
% for i in range(1024):
uniform uint src[${i}] ${"{:#010x}".format(sources[i])}
% endfor

draw rect -1 -1 2 2
probe all rgb 0.0 1.0 0.0
