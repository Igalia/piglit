import re


def emit_test(f, func, input1, input2, expected):
    # Determine the expected return type of the equal function by looking at
    # the string of the expected return value.
    s = expected.split("(")

    spaces = re.sub("[^ ]", " ", func+s[0])

    test = """
[require]
GL >= 2.0
GLSL >= 1.20

[vertex shader file]
glsl-mvp.vert

[fragment shader]
#version 120
void main()
{
  const %s res = %s(%s,
                %s%s);
  gl_FragColor = (res == %s)
    ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}

[test]
clear color 0.0 0.0 0.0 0.0
clear
ortho
draw rect 10 10 10 10
probe rgb 15 15 0.0 1.0 0.0
""" % (s[0], func, input1, spaces, input2, expected)
    f.write(test)


test_vectors = [
    [
        "vec2(3.0, 3.14)",
        "vec2(-6.0, 7.88)",
        "bvec2(false, false)"
        ],
    [
        "vec3(13.4, -0.9, 12.55)",
        "vec3(13.4, 12.0, -55.3)",
        "bvec3(true, false, false)"
        ],
    [
        "vec4(-2.0, 0.0, 0.123, -1000.5)",
        "vec4(-2.4, 0.0, 0.456, 12.5)",
        "bvec4(false, true, false, false)"
        ],
    [
        "ivec2(-8, 12)",
        "ivec2(-19, 12)",
        "bvec2(false, true)"
        ],
    [
        "ivec3(0, 8, 89)",
        "ivec3(4, -7, 33)",
        "bvec3(false, false, false)"
        ],
    [
        "ivec4(11, 1000, 1, -18)",
        "ivec4(55, 1000, -21, -17)",
        "bvec4(false, true, false, false)"
        ],
    [
        "bvec2(true, false)",
        "bvec2(true, true)",
        "bvec2(true, false)"
        ],
    [
        "bvec3(false, true, false)",
        "bvec3(false, false, true)",
        "bvec3(true, false, false)"
        ],
    [
        "bvec4(true, false, false, true)",
        "bvec4(true, true, false, false)",
        "bvec4(true, false, true, false)"
        ]
    ]

test_id = 2
for x in test_vectors:
    name = "glsl-const-builtin-%s-%02d.shader_test" % ("equal", test_id)
    test_id = test_id + 1
    f = open(name, "w")
    emit_test(f, "equal", x[0], x[1], x[2])
    f.close()


test_id = 2
for x in test_vectors:
    name = "glsl-const-builtin-%s-%02d.shader_test" % ("notEqual", test_id)
    test_id = test_id + 1

    # When generating the notEqual tests, each of the values in the expected
    # result vector need to be inverted

    expected = re.sub("true", "FALSE", x[2])
    expected = re.sub("false", "TRUE", expected)
    expected = expected.lower()

    f = open(name, "w")
    emit_test(f, "notEqual", x[0], x[1], expected)
    f.close()
