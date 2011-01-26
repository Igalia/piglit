// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

/*
 * GLSL 1.20
 * 6.1.1 Function Calling Conventions
 * Recursion is not allowed, not even statically.
 */

#version 120

int A()
{
    return A();
}

void main()
{
    gl_Position = gl_Vertex;
}
