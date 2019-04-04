/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

flat varying unsigned int u1[2];
flat varying uvec2 u2[2];
flat varying uvec3 u3[2];
flat varying uvec4 u4[2];

flat varying int i1[2];
flat varying ivec2 i2[2];
flat varying ivec3 i3[2];
flat varying ivec4 i4[2];

void main()
{
    gl_FragColor = vec4(1.0);
}
