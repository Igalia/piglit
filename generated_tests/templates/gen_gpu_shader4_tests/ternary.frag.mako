/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

flat varying ${type1} x;
flat varying ${type2} y, z;

void main()
{
  ${type1} v = ${param.func}(x, y, z);
  gl_FragColor = vec4(v${swizzle});
}
