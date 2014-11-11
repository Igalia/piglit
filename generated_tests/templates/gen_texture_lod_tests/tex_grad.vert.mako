/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_ARB_shader_texture_lod : require

uniform sampler${param.dimensions} s;
attribute vec4 pos;
attribute ${param.coord} coord;
attribute ${param.grad} dPdx;
attribute ${param.grad} dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = ${param.mode}GradARB(s, coord, dPdx, dPdy);
}
