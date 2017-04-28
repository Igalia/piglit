// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_arrays_of_arrays: enable

struct s {
	sampler2D tex[3][2];
	int y;
};

out vec4 color;

void main()
{
	s a[2][4];
	color = texture2D(a[0][0].tex[0][0], vec2(0, 0));
}
