/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler1D s;
varying float coord;
varying float lod;

void main()
{
  gl_FragColor = texture1DLod(s, coord, lod);
}
