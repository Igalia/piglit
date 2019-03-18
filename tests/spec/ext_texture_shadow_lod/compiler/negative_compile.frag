/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * require_extensions: GL_EXT_gpu_shader4 GL_EXT_texture_shadow_lod
 * [end config]
 */

/* The extension is supported by the implementation, but it is not enabled in
 * the shader. This should fail to compile.
 */

#version 130

uniform vec4 a_uniform;
uniform sampler2DArrayShadow sampler;

void main() {
	float color = textureLod(sampler, a_uniform, 1.0);
	gl_FragColor = vec4(color, color, color, color);
}
