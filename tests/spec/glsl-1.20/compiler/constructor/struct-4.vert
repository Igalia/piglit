// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/* FAIL */
#version 120
struct s1 { float f; };

void main()
{
    s1 t = s1(s1(1)); // Only Section 4.1.10 “Implicit Conversions.” are allowed
}
