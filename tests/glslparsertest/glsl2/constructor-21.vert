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
   const mat2 m2 = mat2(2.00, 2.01, 2.10, 2.11);
   const mat4 m4 = mat4(m2);
   gl_Position = m4[0];
}
