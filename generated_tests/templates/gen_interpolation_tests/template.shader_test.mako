%if args.builtin_variable:
# Test proper interpolation of ${args.vs_variable}
%else:
# Test proper interpolation of a non-built-in variable
%endif
%if args.interpolation_qualifier:
# When qualified with ${args.interpolation_qualifier}
%else:
# When no interpolation qualifier present
%endif
# And ShadeModel is ${args.shade_model}
%if args.clipping == 'fixed':
# And clipping via fixed planes
%elif args.clipping == 'vertex':
# And clipping via gl_ClipVertex
%elif args.clipping == 'distance':
# And clipping via gl_ClipDistance
%endif

[require]
GLSL >= ${args.glsl_version}

[vertex shader]
${args.vs_input} vec4 vertex;
${args.vs_input} vec4 input_data;
% if args.interpolation_qualifier or not args.builtin_variable:
${args.interpolation_qualifier} ${args.vs_output} vec4 ${args.vs_variable};
% endif

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
    ${args.vs_variable} = input_data;
  %if args.clipping == 'distance':
    gl_ClipDistance[0] = -1.75 - vertex.z;
  %elif args.clipping == 'vertex':
    gl_ClipVertex = vertex;
  %endif
}

[fragment shader]
% if args.interpolation_qualifier or not args.builtin_variable:
${args.interpolation_qualifier} ${args.fs_input} vec4 ${args.fs_variable};
% endif

void main()
{
    gl_FragColor = ${args.fs_variable};
}

[vertex data]
% for v in args.vertex_data():
${v}
% endfor

[test]
frustum -${args.frustum_near} ${args.frustum_near} -${args.frustum_near} ${args.frustum_near} ${args.frustum_near} ${args.frustum_far}
clear color 0.0 0.0 0.0 0.0
clear
enable GL_VERTEX_PROGRAM_TWO_SIDE
shade model ${args.shade_model}

% if args.clipping == 'distance':
enable GL_CLIP_PLANE0
% elif args.clipping == 'vertex':
enable GL_CLIP_PLANE0
clip plane 0 0.0 0.0 -1.0 -1.75
% endif

draw arrays GL_TRIANGLES 0 3

% for x, y, r, g, b, a in args.probe_data():
relative probe rgba (${x}, ${y}) (${r}, ${g}, ${b}, ${a})
% endfor
