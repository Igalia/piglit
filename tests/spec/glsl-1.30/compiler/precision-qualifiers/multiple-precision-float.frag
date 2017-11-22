/* [config]
 * expect_result: pass
 * glsl_version: 1.30
 * [end config]
 */

#version 130
highp float f1;
mediump float f2;
lowp float f3;
precision mediump float;
precision lowp int;
precision highp float;
void main()
{
	gl_FragColor = vec4(1);
}
