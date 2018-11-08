#!/usr/bin/env python
# coding=utf-8

one_parameter_110 = [
    "radians",
    "degrees",
    "sin",
    "cos",
    "tan",
    "asin",
    "acos",
    "atan",
    "exp",
    "log",
    "exp2",
    "log2",
    "sqrt",
    "inversesqrt",
    "abs",
    "sign",
    "floor",
    "ceil",
    "fract",
    "length",
    "normalize",
    ]

one_parameter_110_fs = [
    "dFdx",
    "dFdy",
    "fwidth",
    ]

two_parameter_110 = [
    "atan",
    "pow",
    "mod",
    "min",
    "max",
    "step",
    "distance",
    "dot",
    "reflect",
    ]

three_parameter_110 = [
    "clamp",
    "mix",
    "smoothstep",
    "faceforward",
    ]

misc_parameter_110 = [
    ["genType", "mod", "genType", "float"],
    ["genType", "min", "genType", "float"],
    ["genType", "max", "genType", "float"],
    ["genType", "clamp", "genType", "float", "float"],
    ["genType", "mix", "genType", "genType", "float"],
    ["genType", "step", "float", "genType"],
    ["genType", "smoothstep", "float", "float", "genType"],
    ["genType", "refract", "genType", "genType", "float"],
    ["float", "noise1", "genType"],
    ["vec2", "noise2", "genType"],
    ["vec3", "noise3", "genType"],
    ["vec4", "noise4", "genType"],
    ]

vec_parameter_110 = [
    ["bvec", "lessThan", "vecType", "vecType"],
    ["bvec", "lessThanEqual", "vecType", "vecType"],
    ["bvec", "greaterThan", "vecType", "vecType"],
    ["bvec", "greaterThanEqual", "vecType", "vecType"],
    ["bvec", "equal", "vecType", "vecType"],
    ["bvec", "notEqual", "vecType", "vecType"],
    ]

bvec_parameter_110 = [
    ["bvec", "equal", "bvec", "bvec"],
    ["bvec", "notEqual", "bvec", "bvec"],
    ["bool", "any", "bvec"],
    ["bool", "all", "bvec"],
    ["bvec", "not", "bvec"],
    ]

genType = ["float", "vec2", "vec3", "vec4"]
vecType = ["vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4"]
bvecType = ["bvec2", "bvec3", "bvec4"]


def typename(t, gen):
    if (t == "genType"):
        return gen
    elif (t == "vecType"):
        return gen
    elif (t == "bvec"):
        return "bvec" + gen[len(gen) - 1]
    else:
        return t


def emit_pattern_call(f, pat_list, type_list):
    for sig in pat_list:
        for t in type_list:
            name = sig[1]
            ret = typename(sig[0], t)

            params = "u_%s" % (typename(sig[2], t))
            for p in sig[3:]:
                params += ", u_%s" % (typename(p, t))

            f.write("  t_%s = %s(%s);\n" % (ret, name, params))

    f.write("\n")


def emit_matrixCompMult(f, matTypes):
    for mat in matTypes:
        f.write("  t_%s = matrixCompMult(u_%s, u_%s);\n" % (mat, mat, mat))


def emit_110_tests(f):
    f.write("/* PASS */\n#version 110\n\n")
    f.write("uniform float u_float;\n")

    for s in ["vec", "ivec", "bvec", "mat"]:
        for i in [2, 3, 4]:
            f.write("uniform %s%d u_%s%d;\n" % (s, i, s, i))

    f.write("""\nvoid main()
{
  gl_Position = gl_Vertex;

  float t_float = float(0.0);
  bool t_bool = false;\n""")

    for i in [2, 3, 4]:
        f.write("  vec%d  t_vec%d  = vec%d (0.0);\n" % (i, i, i))
        f.write("  bvec%d t_bvec%d = bvec%d(0.0);\n" % (i, i, i))
        f.write("  mat%d  t_mat%d  = mat%d (0.0);\n" % (i, i, i))

    f.write("\n")

    for name in one_parameter_110:
        for t in genType:
            f.write("  t_%s += %s(u_%s);\n" % (t, name, t))

    f.write("\n")

    for name in two_parameter_110:
        for t in genType:
            f.write("  t_%s += %s(u_%s, -u_%s);\n" % (t, name, t, t))

    f.write("\n")

    for name in three_parameter_110:
        for t in genType:
            f.write("  t_%s += %s(u_%s, -u_%s, 11.7 * u_%s);\n" %
                    (t, name, t, t, t))

    f.write("\n")

    emit_pattern_call(f, misc_parameter_110, genType)
    emit_pattern_call(f, vec_parameter_110, vecType)
    emit_pattern_call(f, bvec_parameter_110, bvecType)
    emit_matrixCompMult(f, ["mat2", "mat3", "mat4"])

    f.write("}\n")


def emit_110_fs_tests(f):
    f.write("/* PASS */\n#version 110\n\n")
    f.write("uniform float u_float;\n")

    for i in [2, 3, 4]:
        f.write("uniform vec%d u_vec%d;\n" % (i, i))

    f.write("""\nvoid main()
{
  gl_FragColor = gl_Color;

  float t_float = float(0.0);\n""")

    for i in [2, 3, 4]:
        f.write("  vec%d  t_vec%d  = vec%d (0.0);\n" % (i, i, i))

    f.write("\n")

    for name in one_parameter_110_fs:
        for t in genType:
            f.write("  t_%s = %s(u_%s);\n" % (t, name, t))

    f.write("}\n")


def emit_120_tests(f):
    f.write("/* PASS */\n#version 120\n\n")

    for cols in [2, 3, 4]:
        f.write("uniform mat%d u_mat%d;\n" % (cols, cols))
        f.write("uniform vec%d u_vec%d;\n" % (cols, cols))

        for rows in [2, 3, 4]:
            mat = "mat%dx%d" % (cols, rows)
            f.write("uniform %s u_%s;\n" % (mat, mat))

    f.write("""\nvoid main()
{
  gl_Position = gl_Vertex;

""")

    for cols in [2, 3, 4]:
        f.write("  mat%d   t_mat%d   = mat%d  (0.0);\n" % (cols, cols, cols))

        for rows in [2, 3, 4]:
            mat = "mat%dx%d" % (cols, rows)
            f.write("  %s t_%s = %s(0.0);\n" % (mat, mat, mat))

    f.write("\n")

    for cols in [2, 3, 4]:
        dst = "t_mat%d" % (cols)
        src = "u_mat%d" % (cols)
        f.write("  %s   = transpose(%s);\n" % (dst, src))

        for rows in [2, 3, 4]:
            dst = "t_mat%dx%d" % (cols, rows)
            src = "u_mat%dx%d" % (rows, cols)
            f.write("  %s = transpose(%s);\n" % (dst, src))

    f.write("\n")

    for cols in [2, 3, 4]:
        f.write("  t_mat%d   = outerProduct(u_vec%d, u_vec%d);\n"
                % (cols, cols, cols))

        for rows in [2, 3, 4]:
            f.write("  t_mat%dx%d = outerProduct(u_vec%d, u_vec%d);\n" %
                    (cols, rows, rows, cols))

    f.write("\n")

    emit_matrixCompMult(f, ["mat2x2", "mat2x3", "mat2x4",
                            "mat3x2", "mat3x3", "mat3x4",
                            "mat4x2", "mat4x3", "mat4x4"])

    f.write("}\n")


f = open("builtin-functions-110.vert", "w")
emit_110_tests(f)
f.close()

f = open("builtin-functions-110.frag", "w")
emit_110_fs_tests(f)
f.close()

f = open("builtin-functions-120.vert", "w")
emit_120_tests(f)
f.close()
