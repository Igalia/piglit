/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod GL_ARB_texture_rectangle
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2DRectShadow s;
attribute vec4 pos;
attribute vec4 coord;
attribute vec2 dPdx;
attribute vec2 dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = shadow2DRectProjGradARB(s, coord, dPdx, dPdy);
}
