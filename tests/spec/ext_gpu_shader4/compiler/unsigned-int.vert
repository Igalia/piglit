/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

attribute vec4 pos;
attribute unsigned int x;
uniform unsigned int w;
flat varying unsigned int v;

void main()
{
  gl_Position = pos;
  int i = int(pos.y);
  unsigned int t = 1u;
  unsigned int u = unsigned int(pos.x);
  unsigned int v = unsigned int(i);
  v = t + unsigned int(x + 2U) + u + v + w;
}
