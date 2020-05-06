// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_storage_buffer_object GL_ARB_gpu_shader5
// [end config]

/* Per GLSL 4.60 spec, section 8.11 - all atomicOP functions have first
 * parameter with "inout" qualifier.
 * Per section 6.1 - for "inout" parameter to be implicitly converted, its type
 * must support bi-derectional conversion. There is no such types in glsl.
 */

#version 150
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_ARB_gpu_shader5 : require

buffer bufblock {
  ivec3 var;
};

void main()
{
  atomicMin(var.x, 1u);
  gl_Position = vec4(0.0);
}
