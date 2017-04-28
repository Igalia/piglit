// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

struct foo {
	float x;
	sampler2D tex;
};

uniform Block {
   foo f;
};

out vec4 color;

void main()
{
	color = texture2D(f.tex, vec2(0, 0));
}
