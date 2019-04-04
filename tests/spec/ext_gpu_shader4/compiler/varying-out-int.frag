/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

flat varying int i1;
flat varying ivec2 i2;
flat varying ivec3 i3;
flat varying ivec4 i4;
flat varying unsigned int u1;
flat varying uvec2 u2;
flat varying uvec3 u3;
flat varying uvec4 u4;

varying out unsigned int o1;
varying out uvec2 o2;
varying out uvec3 o3;
varying out uvec4 o4;
varying out int o5;
varying out ivec2 o6;
varying out ivec3 o7;
varying out ivec4 o8;

void main()
{
    o1 = u1;
    o2 = u2;
    o3 = u3;
    o4 = u4;
    o5 = i1;
    o6 = i2;
    o7 = i3;
    o8 = i4;
}
