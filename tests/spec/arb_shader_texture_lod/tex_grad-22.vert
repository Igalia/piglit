/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler1DShadow s;
attribute vec4 pos;
attribute vec4 coord;
attribute float dPdx;
attribute float dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = shadow1DProjGradARB(s, coord, dPdx, dPdy);
}
