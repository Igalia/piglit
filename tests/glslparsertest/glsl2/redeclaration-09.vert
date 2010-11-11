// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - variables and functions have separate namespaces in 1.10.
 *
 * See also redeclaration-02.vert (other order) and redeclaration-10.vert
 * (this fails in 1.20 due to shared namespaces).
 */
float foo()
{
    return 0.5;
}

const float foo = 1.0;

void main()
{
    gl_Position = vec4(foo - foo());
}
