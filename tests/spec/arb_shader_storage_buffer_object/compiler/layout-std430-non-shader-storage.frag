// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object GL_ARB_uniform_buffer_object
// [end config]

/* From the GL_ARB_shader_storage_buffer_object:
 *
 *     "The std430 qualifier is supported only for shader storage blocks;
 *      a shader using the std430 qualifier on a uniform block will fail to
 *      compile."
 */

#version 120
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_ARB_uniform_buffer_object: require

layout(std430) uniform Ubo {
   vec4 b;
};

vec4 foo(void) {
	return b;
}
