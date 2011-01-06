/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2D s;
attribute vec4 pos;
attribute vec2 coord;
attribute vec2 dPdx;
attribute vec2 dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = texture2DGradARB(s, coord, dPdx, dPdy);
}
