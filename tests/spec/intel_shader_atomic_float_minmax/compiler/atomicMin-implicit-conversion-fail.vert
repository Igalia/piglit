// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_storage_buffer_object GL_INTEL_shader_atomic_float_minmax
// [end config]

/* All atomicOP functions from GL_INTEL_shader_atomic_float_minmax have first
 * parameter with "inout" qualifier.
 * Per section 6.1 - for "inout" parameter to be implicitly converted, its type
 * must support bi-directional conversion. There is no such types in glsl.
 */

#version 150
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_INTEL_shader_atomic_float_minmax : require

buffer bufblock {
  int var;
};

void main()
{
  atomicMin(var, 1.0);
  gl_Position = vec4(0.0);
}
