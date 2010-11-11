// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

vec2 func()
{
    vec2 v;
    return v;
}

void main()
{
    const vec3 v = vec3(1.0, func()); // user defined functions do not return const value
    gl_FragColor = vec4(v, v);
}
