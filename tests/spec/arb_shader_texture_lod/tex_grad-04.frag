/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler1DShadow s;
varying vec3 coord;
varying float dPdx;
varying float dPdy;

void main()
{
  gl_FragColor = shadow1DGradARB(s, coord, dPdx, dPdy);
}
