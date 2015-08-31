// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_texture_image_samples
// [end config]

#version 150
#extension GL_ARB_shader_texture_image_samples: require

uniform sampler2DMS s2D;
uniform sampler2DMSArray s2DArray;
uniform isampler2DMS is2D;
uniform isampler2DMSArray is2DArray;
uniform usampler2DMS us2D;
uniform usampler2DMSArray us2DArray;

void main()
{
	int res = 0;

	res += textureSamples(s2D);
	res += textureSamples(s2DArray);
	res += textureSamples(is2D);
	res += textureSamples(is2DArray);
	res += textureSamples(us2D);
	res += textureSamples(us2DArray);

	gl_FragColor = vec4(float(res));
}
