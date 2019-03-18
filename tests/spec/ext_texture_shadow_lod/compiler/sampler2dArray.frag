/* [config]
 * expect_result: pass
 * glsl_version: 1.30
 * require_extensions: GL_EXT_texture_shadow_lod
 * [end config]
 */

#version 130
#extension GL_EXT_texture_shadow_lod : require

uniform vec4 a_uniform;
uniform sampler2DArrayShadow array_shadow;

void main() {
	float color = texture(array_shadow, a_uniform, 1.0);
	color += texture(array_shadow, a_uniform);
	color += textureOffset(array_shadow, a_uniform, ivec2(4,4), 1.0);
	color += textureOffset(array_shadow, a_uniform, ivec2(4,4));
	color += textureLod(array_shadow, a_uniform, 1.0);
	color += textureLodOffset(array_shadow, a_uniform, 1.0, ivec2(4,4));
	gl_FragColor = vec4(color, color, color, color);
}
