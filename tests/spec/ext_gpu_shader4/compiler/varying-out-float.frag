/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

flat varying float u1;
flat varying vec2 u2;
flat varying vec3 u3;
flat varying vec4 u4;

varying out float o1;
varying out vec2 o2;
varying out vec3 o3;
varying out vec4 o4;

void main()
{
    o1 = u1;
    o2 = u2;
    o3 = u3;
    o4 = u4;
}
