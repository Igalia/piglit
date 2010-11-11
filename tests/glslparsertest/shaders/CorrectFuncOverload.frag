// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

void testVoid (vec4 v, vec4 v1)
{
}

void testVoid (ivec4 v, ivec4 v1)
{
}

void main(void)
{
    vec4 v;
    ivec4 i;
    testVoid(i, i);
    testVoid(v, v);
    gl_FragColor = v;
}
