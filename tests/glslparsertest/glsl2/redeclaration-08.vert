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

