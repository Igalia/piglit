/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

attribute vec4 pos;
attribute ${type1} x;
flat varying ${type1} v;

void main()
{
  gl_Position = pos;
  v = ${param.op}x;
}
