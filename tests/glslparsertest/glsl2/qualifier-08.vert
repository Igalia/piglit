// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - centroid and invariant are not reserved words in GLSL 1.10 */
uniform vec2 centroid;
uniform vec2 invariant;

void main()
{
  gl_Position = vec4(centroid, invariant);
}
