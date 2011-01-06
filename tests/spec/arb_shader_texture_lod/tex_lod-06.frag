/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2D s;
varying vec2 coord;
varying float lod;

void main()
{
  gl_FragColor = texture2DLod(s, coord, lod);
}
