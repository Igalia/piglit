// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - like struct-01.vert, but now the other comes first */

struct foo {
    float f;
    int i;
    bool b;
};

vec4 foo(vec4 a, vec4 b)
{
    return vec4(dot(a, b));
}

