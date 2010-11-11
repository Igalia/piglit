// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 120
void main()
{
   const mat3 m3 = mat3(3.00, 3.01, 3.02, 3.10, 3.11, 3.12, 3.20, 3.21, 3.22);
   const mat2 m2 = mat2(m3);
   gl_Position = vec4(m2);
}
