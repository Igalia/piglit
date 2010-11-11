// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - structure type name conflicts with a variable name in same scope */
void main()
{
    float foo;
    struct foo {
       bvec4 bs;
    };

    gl_Position = vec4(0.0);
}
