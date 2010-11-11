// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 120
/* PASS */

uniform mat3 a;

void main()
{
    mat2 m;

    m = mat2(a);
    gl_Position = gl_Vertex;
}
