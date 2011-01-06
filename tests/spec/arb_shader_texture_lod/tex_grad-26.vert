/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2DShadow s;
attribute vec4 pos;
attribute vec3 coord;
attribute vec2 dPdx;
attribute vec2 dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = shadow2DGradARB(s, coord, dPdx, dPdy);
}
