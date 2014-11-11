/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_ARB_shader_texture_lod : require

uniform sampler${param.dimensions} s;
varying ${param.coord} coord;
varying ${param.grad} dPdx;
varying ${param.grad} dPdy;

void main()
{
  gl_FragColor = ${param.mode}GradARB(s, coord, dPdx, dPdy);
}
