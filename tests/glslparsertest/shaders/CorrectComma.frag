// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

struct s {
    float f;
    vec3 v;
};

void main()
{
    const vec4 v = (vec4(1,2,3,4), vec4(5,6,7,8));  // 5,6,7,8
    const s s1 = (s(9.0, vec3(10,11,12)), s(13.0, vec3(14,15,16))); // 13,14,15,16
    gl_FragColor = v + vec4(s1.f, s1.v);  // 18, 20, 22, 24
}
