/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler1DShadow s;
attribute vec4 pos;
attribute vec3 coord;
attribute float dPdx;
attribute float dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = shadow1DGradARB(s, coord, dPdx, dPdy);
}
