/* This is the passthrough vertex shader used by the shader_runner in SPIR-V
 * mode. It is compiled to SPIR-V at build time.
 *
 * glslangValidator -G -o shader_runner_vs_passthrough.spv shader_runner_vs_passthrough.vert
 * xxd -i shader_runner_vs_passthrough.spv > shader_runner_vs_passthrough.h
 */
#version 450

layout(location = 0) in vec4 piglit_vertex;

void main()
{
    gl_Position = piglit_vertex;
}
