// [config]
// expect_result: fail
// glsl_version: 1.20
//
// [end config]

/* FAIL */
#version 120
struct s1 { float f; };

struct s2 { s1 f; };

void main()
{
    s2 t = s2(1);
}
