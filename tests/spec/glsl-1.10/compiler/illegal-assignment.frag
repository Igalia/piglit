/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */

void main()
{
	float x = main;
	gl_FragColor = vec4(0.5);
}
