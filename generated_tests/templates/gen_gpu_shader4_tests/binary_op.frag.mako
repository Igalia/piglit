/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

flat varying ${types[0]} x;
flat varying ${types[1]} y, z;

void main()
{
  ${result_type} v = x ${param.op} y;
  v ${param.op}= z;
  gl_FragColor = vec4(v${swizzle});
}
