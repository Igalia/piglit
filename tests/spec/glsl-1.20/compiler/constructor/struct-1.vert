// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/* FAIL */
#version 120
struct s { vec2 f; };

void main()
{
    s t = s(vec3(1.0, 3.0, 0.0)); // Only Section 4.1.10 “Implicit Conversions.” are allowed
}
