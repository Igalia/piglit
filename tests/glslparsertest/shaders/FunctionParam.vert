// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

int y = 1;

int foo(int, int b[y])  // array size should be constant
{
    return 1;
}

void main()
{
    int a[1];

    gl_Position = vec4(1.0);
}
