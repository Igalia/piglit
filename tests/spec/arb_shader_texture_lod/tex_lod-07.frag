/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2D s;
varying vec3 coord;
varying float lod;

void main()
{
  gl_FragColor = texture2DProjLod(s, coord, lod);
}
