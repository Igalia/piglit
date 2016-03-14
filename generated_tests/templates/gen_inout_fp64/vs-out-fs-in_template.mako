# test emitting a ${type_name} from vs->fs works
# when originally written this failed in varying lowering

[require]
${require}\

[vertex shader]
${preprocessor}\

uniform double arg0;

in vec4 vertex;
flat out ${type_name} dout1;

void main()
{
    gl_Position = vertex;
    dout1 = ${type_name}(arg0);
}

[fragment shader]
${preprocessor}\

flat in ${type_name} dout1;
uniform double tolerance;
uniform double expected;
out vec4 color;

void main()
{
    ${type_name} result = trunc(dout1);
    color = distance(result, ${type_name}(expected)) <= tolerance ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}

[vertex data]
vertex/float/2
-1.0 -1.0
 1.0 -1.0
 1.0  1.0
-1.0  1.0

[test]
uniform double arg0 1.7976931348623157E+308
uniform double expected 1.7976931348623157E+308
uniform double tolerance 2.0000000000000002e-05
draw arrays GL_TRIANGLE_FAN 0 4
probe rgba 0 0 0.0 1.0 0.0 1.0
