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
   const mat3x2 c1 = mat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
   const mat4x3 c2 = mat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
                            9.0, 10.0, 11.0, 12.0);
   /* Should be:
    * 22 49 76  103
    * 28 64 100 136
    */

   const mat4x2 m = c1 * c2;
}

