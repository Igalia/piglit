/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */

// Test using a non-existant function.  Should not compile.
#extension GL_FOO_bar: require
void main()
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);
}
