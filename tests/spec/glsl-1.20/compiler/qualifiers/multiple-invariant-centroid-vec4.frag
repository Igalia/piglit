/* [config]
 * expect_result: pass
 * glsl_version: 1.20
 * [end config]
 */

#version 120
invariant varying vec4 v1;
centroid varying vec4 v2;
invariant centroid varying vec4 v3;
varying vec4 v4;
invariant v4;
void main()
{
	gl_FragColor = vec4(1);
}
