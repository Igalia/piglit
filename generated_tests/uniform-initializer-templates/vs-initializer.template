[require]
GLSL >= ${"{0}.{1}".format(major, minor)}

[vertex shader]
varying vec4 color;

% for (type, name, value) in type_list:
uniform ${type} ${name} = ${value};
% endfor

void main()
{
  if (${"\n      && ".join('{0} == {1}'.format(name, value) for type, name, value in type_list)}) {
    color = vec4(0, 1, 0, 1);
  } else {
    color = vec4(1, 0, 0, 1);
  }

  gl_Position = gl_Vertex;
}

[fragment shader]
varying vec4 color;

void main()
{
  gl_FragColor = color;
}

[test]
draw rect -1 -1 2 2
probe all rgb 0 1 0
