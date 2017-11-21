/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 *
 * Test for bug in TIntermediate::addUnaryMath?
 */

void main()
{
	-vec4(x ? 1.0 : -1.0);
}
