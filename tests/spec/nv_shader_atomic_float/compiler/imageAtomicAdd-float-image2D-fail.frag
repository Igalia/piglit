// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_NV_shader_atomic_float GL_ARB_shader_image_load_store
// [end config]
//
// The extension is supported by the implementation, but it is not enabled in
// the shader.  This should fail to compile.

#version 150
#extension GL_ARB_shader_image_load_store: require

layout(r32f) uniform image2D img;
uniform float v;
out vec4 color;

void main()
{
        color = vec4(imageAtomicAdd(img, ivec2(gl_FragCoord.xy), v));
}
