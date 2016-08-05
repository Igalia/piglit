// [config]
// expect_result: pass
// glsl_version: 1.20
// [end config]

/* PASS */
#version 120
struct s1 { float f; };

struct s2 { s1 g; };

void main()
{
    s2 t = s2(s1(1)); // an implicit conversion should happen here
}
