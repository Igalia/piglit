/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler3D s;
attribute vec4 pos;
attribute vec3 coord;
attribute vec3 dPdx;
attribute vec3 dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = texture3DGradARB(s, coord, dPdx, dPdy);
}
