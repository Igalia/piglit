<%!
def constructor(type, values):
    return "{0}({1})".format(type, ", ".join(values))

%>[require]
GLSL >= ${"{0}.{1}".format(major, minor)}

[vertex shader]

void main()
{
  gl_Position = gl_Vertex;
}

[fragment shader]

% for (type, name, value) in type_list:
uniform ${type} ${name} = ${value};
% endfor

void main()
{
  if (${"\n      && ".join('{0} == {1}'.format(type_list[i][1], constructor(type_list[i][0], api_types[i][2])) for i in range(1, len(type_list) - 1))}) {
    gl_FragColor = vec4(0, 1, 0, 1);
  } else {
    gl_FragColor = vec4(1, 0, 0, 1);
  }
}

[test]
% for (api_type, name, values) in api_types:
uniform ${api_type} ${name} ${" ".join(values)}
% endfor
draw rect -1 -1 2 2
probe all rgb 0 1 0
