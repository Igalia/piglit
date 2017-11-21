/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */

void main()
{
	vec3 v;
	if (v) {
	}
	gl_FragColor = vec4(0.5);
}
