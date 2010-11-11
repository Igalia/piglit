// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS
 *
 * In a function prototype it is valid to have some parameters named
 * and some parameters not named.
 */

void bar(int x, float);

void main()
{
  gl_Position = vec4(0.0);
}
