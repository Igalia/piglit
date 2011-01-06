/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler1D s;
varying vec4 coord;
varying float lod;

void main()
{
  gl_FragColor = texture1DProjLod(s, coord, lod);
}
