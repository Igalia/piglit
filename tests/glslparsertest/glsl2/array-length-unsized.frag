// [config]
// expect_result: fail
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL: According to GLSL 1.20 page 20,
 * "The length method cannot be called on an array that has not been
 *  explicitly sized."
 */
#version 120
void main()
{
   float a[];
   a[4] = 4.0;
   float length = a.length();
}
