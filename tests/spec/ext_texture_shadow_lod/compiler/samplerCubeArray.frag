/* [config]
 * expect_result: pass
 * glsl_version: 1.30
 * require_extensions: GL_ARB_texture_cube_map_array GL_EXT_texture_shadow_lod
 * [end config]
 */

#version 130
#extension GL_EXT_texture_shadow_lod : require
#extension GL_ARB_texture_cube_map_array : require

uniform vec4 a_uniform;
uniform samplerCubeArrayShadow cube_shadow;

void main() {
	float color = texture(cube_shadow, a_uniform, 1.0);
	color += texture(cube_shadow, a_uniform, 0.0);
	color += textureLod(cube_shadow, a_uniform, 0.0, 1.0);
	gl_FragColor = vec4(color, color, color, color);
}
