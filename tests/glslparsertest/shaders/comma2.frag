// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

void main()
{
    const vec4 v = (vec4(1,2,3,4), vec4(5,6,7,8), 1.2); // right most value of comma operator shoul be a vec4
    const vec4 v1 = (vec3(0.2, 2.0), vec4(1,2,3,4), vec4(5,6,7,8)); 
    const vec4 v2 = (vec4(1,2,3,4), vec2(2.1, 2),  vec4(5,6,7,8));  
    gl_FragColor = v + v1 + v2;
}
