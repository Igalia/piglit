[require]
GLSL >= ${major}.${minor}

[vertex shader]

void main()
{
  gl_Position = gl_Vertex;
}

[fragment shader]

% for type_, name, value in type_list:
const ${type_} c${name} = ${value};
% endfor

% for type_, name, value in type_list:
uniform ${type_} ${name} = c${name};
% endfor

void main()
{
  if (${" && ".join('{0} == {1}'.format(n, v) for _, n, v in type_list)}) {
    gl_FragColor = vec4(0, 1, 0, 1);
  } else {
    gl_FragColor = vec4(1, 0, 0, 1);
  }
}

[test]
draw rect -1 -1 2 2
probe all rgb 0 1 0
