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
 * This test reproduces the failure reported in bugzilla #29607.
 */
const float exp = 1.0;

void main()
{
  const float exp = 2.0;
  gl_Position = vec4(0.0);
}
