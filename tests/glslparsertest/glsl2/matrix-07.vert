// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS */

uniform mat2 a;
uniform mat2 b;
uniform mat2 c;
uniform mat2 d;
uniform mat3 e;
uniform mat3 f;
uniform mat3 g;
uniform mat3 h;
uniform mat4 i;
uniform mat4 j;
uniform mat4 k;
uniform mat4 l;

void main()
{
    mat2 x;
    mat3 y;
    mat4 z;

    x = a * b + c / d;
    y = e * f + g / h;
    z = i * j + k / l;

    gl_Position = gl_Vertex;
}
