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
   const mat2 c1 = mat2(0.0, -1.0, -1.0, 0.0);
   const mat2 c2 = 0.5 + c1;
   const mat2 c3 = c2 + 0.5;
}
