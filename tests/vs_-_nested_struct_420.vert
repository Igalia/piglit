// [config]
// expect_result: pass
// glsl_version: 4.20
//
// [end config]

/* PASS */
#version 420
struct s1 { float f; };

struct s2 { s1 f; };

void main()
{
    s2 t = { { 1 } }; // an implicit conversion should happen here
}
