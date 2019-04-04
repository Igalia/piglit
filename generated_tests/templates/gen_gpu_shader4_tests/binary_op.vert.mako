/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

attribute vec4 pos;
attribute ${types[0]} x;
attribute ${types[1]} y, z;
flat varying ${result_type} v;

void main()
{
  gl_Position = pos;
  v = x ${param.op} y;
  v ${param.op}= z;
}
