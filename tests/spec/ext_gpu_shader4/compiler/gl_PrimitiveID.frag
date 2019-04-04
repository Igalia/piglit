/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

void main()
{
  int v = gl_PrimitiveID;
  gl_FragColor = vec4(v);
}
