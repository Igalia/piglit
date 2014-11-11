[require]
GLSL >= ${major}.${minor}

[vertex shader]
varying vec4 color;

% for type_, name, _ in type_list:
uniform ${type_} ${name};
% endfor

void main()
{
  if (${" && ".join('{0} == {1}'.format(name, value) for _, name, value in type_list)}) {
    color = vec4(0, 1, 0, 1);
  } else {
    color = vec4(1, 0, 0, 1);
  }

  gl_Position = gl_Vertex;
}

[fragment shader]
varying vec4 color;

% for type_, name, value in type_list:
uniform ${type_} ${name} = ${value};
% endfor

void main()
{
  gl_FragColor = color;
}

[test]
draw rect -1 -1 2 2
probe all rgb 0 1 0
