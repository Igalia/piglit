// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]

#version 150
#extension GL_ARB_tessellation_shader : enable

layout(vertices = 3) out;

void main() {
	gl_out[gl_InvocationID].gl_Position.xy = gl_in[gl_InvocationID].gl_Position.xy;
}
