// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

/* From the GLSL 4.50 spec, section 4.4.5:
 *
 *  "It is a compile-time error to specify the binding identifier for
 *  the global scope or for block member declarations."
 */

#version 150
#extension GL_ARB_shader_storage_buffer_object: require

layout(binding=1) buffer;

void main()
{
}
