/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

attribute vec4 pos;
attribute ${type1} x, y;
flat varying ${type1} v;

void main()
{
  gl_Position = pos;
  ${bvec} b = ${param.func}(x, y);
  v = ${type1}(b);
}
