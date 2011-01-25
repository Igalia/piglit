/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler3D s;
varying vec4 coord;
varying vec3 dPdx;
varying vec3 dPdy;

void main()
{
  gl_FragColor = texture3DProjGradARB(s, coord, dPdx, dPdy);
}
