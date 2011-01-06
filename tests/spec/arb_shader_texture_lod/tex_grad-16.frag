/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod GL_ARB_texture_rectangle
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2DRectShadow s;
varying vec3 coord;
varying vec2 dPdx;
varying vec2 dPdy;

void main()
{
  gl_FragColor = shadow2DRectGradARB(s, coord, dPdx, dPdy);
}
