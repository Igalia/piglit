/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_ARB_shader_texture_lod GL_ARB_texture_rectangle
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler2DRect s;
varying vec2 coord;
varying vec2 dPdx;
varying vec2 dPdy;

void main()
{
  gl_FragColor = texture2DRectGradARB(s, coord, dPdx, dPdy);
}
