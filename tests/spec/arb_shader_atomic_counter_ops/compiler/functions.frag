/* [config]
 * expect_result: pass
 * glsl_version: 1.40
 * require_extensions: GL_ARB_shader_atomic_counter_ops
 * [end config]
 *
 * Check that the builtin functions defined by the extension
 * are present.
 */
#version 140
#extension GL_ARB_shader_atomic_counters: require
#extension GL_ARB_shader_atomic_counter_ops: require

layout (binding=0) uniform atomic_uint c;
out uvec4 fcolor;
uniform uint data, comp;

void main()
{
        fcolor.x = atomicCounterAddARB(c, data) +
                atomicCounterSubtractARB(c, data) +
                atomicCounterMinARB(c, data) +
                atomicCounterMaxARB(c, data) +
                atomicCounterAndARB(c, data) +
                atomicCounterOrARB(c, data) +
                atomicCounterXorARB(c, data) +
                atomicCounterExchangeARB(c, data) +
                atomicCounterCompSwapARB(c, comp, data);
}
