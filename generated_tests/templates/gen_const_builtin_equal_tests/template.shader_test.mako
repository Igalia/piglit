[require]
GLSL >= 1.20

[vertex shader]
void main()
{
  gl_Position = gl_Vertex;
}

[fragment shader]
void main()
{
  const ${expected.split('(')[0]} res = ${func}(${input[0]}, ${input[1]});
  gl_FragColor = (res == ${expected})
    ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}

[test]
draw rect -1 -1 2 2
probe all rgb 0.0 1.0 0.0
