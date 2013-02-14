[require]
GLSL >= ${"{0}.{1}".format(major, minor)}

[vertex shader]

void main()
{
  gl_Position = gl_Vertex;
}

[fragment shader]

% for (type, name, value) in type_list:
const ${type} c${name} = ${value};
% endfor

% for (type, name, value) in type_list:
uniform ${type} ${name} = c${name};
% endfor

void main()
{
  if (${"\n      && ".join('{0} == {1}'.format(name, value) for type, name, value in type_list)}) {
    gl_FragColor = vec4(0, 1, 0, 1);
  } else {
    gl_FragColor = vec4(1, 0, 0, 1);
  }
}

[test]
draw rect -1 -1 2 2
probe all rgb 0 1 0
