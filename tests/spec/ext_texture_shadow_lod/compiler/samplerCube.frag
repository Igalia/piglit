/* [config]
 * expect_result: pass
 * glsl_version: 1.30
 * require_extensions: GL_EXT_texture_shadow_lod
 * [end config]
 */

#version 130
#extension GL_EXT_texture_shadow_lod : require

uniform vec4 a_uniform;
uniform samplerCubeShadow cube_shadow;

void main() {
	float color = textureLod(cube_shadow, a_uniform, 1.0);
	gl_FragColor = vec4(color, color, color, color);
}
