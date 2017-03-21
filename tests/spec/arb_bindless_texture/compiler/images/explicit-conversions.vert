// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable

// The ARB_bindless_texture spec says:
//
//  "Modify Section 5.4.1, Conversion and Scalar Constructors, p. 60"
//
//   (add the following constructors:)
//
//  "In the following four constructors, the low 32 bits of the image
//   type correspond to the .x component of the uvec2 and the high 32 bits
//   correspond to the .y component."
//
//   "Converts an image type to a pair of 32-bit unsigned integers:
//   uvec2(any image type)"
//
//   "Converts a pair of 32-bit unsigned integers to an image type:
//   any image type(uvec2)"

void any_image_type_to_uvec2()
{
	uvec2 pair = uvec2(0, 0);

	writeonly imageBuffer ibuf;
	pair = uvec2(ibuf);

	writeonly image1D i1d;
	pair = uvec2(i1d);

	writeonly image1DArray i1darr;
	pair = uvec2(i1darr);

	writeonly image2D i2d;
	pair = uvec2(i2d);

	writeonly image2DMS i2dms;
	pair = uvec2(i2dms);

	writeonly image2DArray i2darr;
	pair = uvec2(i2darr);

	writeonly image2DMSArray i2dmsarr;
	pair = uvec2(i2dmsarr);

	writeonly image2DRect i2drect;
	pair = uvec2(i2drect);

	writeonly image3D i3d;
	pair = uvec2(i3d);

	writeonly imageCube icube;
	pair = uvec2(icube);

	writeonly imageCubeArray icubearr;
	pair = uvec2(icubearr);
}

void uvec2_to_any_image_type()
{
	uvec2 pair = uvec2(0, 0);

	writeonly imageBuffer ibuf = imageBuffer(pair);
	writeonly image1D i1d = image1D(pair);
	writeonly image1DArray i1darr = image1DArray(pair);
	writeonly image2D i2d = image2D(pair);
	writeonly image2DMS i2dms = image2DMS(pair);
	writeonly image2DArray i2darr = image2DArray(pair);
	writeonly image2DMSArray i2dmsarr = image2DMSArray(pair);
	writeonly image2DRect i2drect =  image2DRect(pair);
	writeonly image3D i3d = image3D(pair);
	writeonly imageCube icube = imageCube(pair);
	writeonly imageCubeArray icubearr = imageCubeArray(pair);
}

void main()
{
	any_image_type_to_uvec2();
	uvec2_to_any_image_type();
}
