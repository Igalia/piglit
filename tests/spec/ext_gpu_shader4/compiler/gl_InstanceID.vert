/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

attribute vec4 pos;
flat varying int v;

void main()
{
  gl_Position = pos;
  v = gl_InstanceID;
}
