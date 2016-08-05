// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/* FAIL */
#version 120
struct s1 { float f; };

struct s2 { s1 g; };

void main()
{
    s2 t = s2(s2(s1(1))); // Only Section 4.1.10 “Implicit Conversions.” are allowed
}
