/* FAIL
 * Without GLSL 1.40 or GL_ARB_shader_texture_lod, the "Lod" versions of the
 * texture lookup functions are not available in fragment shaders.
 */
uniform sampler1D s;
varying vec2 coord;
varying float lod;

void main()
{
  gl_FragColor = texture1DProjLod(s, coord, lod);
}
