// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

void main()
{
    vec4 v1 = vec4(5,6,7,8);
    vec4 v2 = vec4(9,10, 11, 12);
    vec3 v3 = (v1 * v2).ywx;
    float f = (v2 * v1).z;
    vec3 v4 = normalize((v1.ywx * v3).xyz).xyz;
    gl_Position = vec4(v4, f);
}
