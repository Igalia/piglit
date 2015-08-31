// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_image_load_store GL_ARB_shader_texture_image_samples
// [end config]

#version 150
#extension GL_ARB_shader_image_load_store: require
#extension GL_ARB_shader_texture_image_samples: require

writeonly uniform image2DMS i2D;
writeonly uniform image2DMSArray i2DArray;
writeonly uniform iimage2DMS ii2D;
writeonly uniform iimage2DMSArray ii2DArray;
writeonly uniform uimage2DMS ui2D;
writeonly uniform uimage2DMSArray ui2DArray;

void main()
{
	int res = 0;

	res += imageSamples(i2D);
	res += imageSamples(i2DArray);
	res += imageSamples(ii2D);
	res += imageSamples(ii2DArray);
	res += imageSamples(ui2D);
	res += imageSamples(ui2DArray);

	gl_FragColor = vec4(float(res));
}
