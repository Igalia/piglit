// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_texture_cube_map_array: enable

// The ARB_bindless_texture spec says:
//
//  "Modify Section 5.4.1, Conversion and Scalar Constructors, p. 60"
//
//   (add the following constructors:)
//
//  "In the following four constructors, the low 32 bits of the sampler
//   type correspond to the .x component of the uvec2 and the high 32 bits
//   correspond to the .y component."
//
//   "Converts a sampler type to a pair of 32-bit unsigned integers:
//   uvec2(any sampler type)"
//
//   "Converts a pair of 32-bit unsigned integers to a sampler type:
//   any sampler type(uvec2)"

void any_sampler_type_to_uvec2()
{
	uvec2 pair = uvec2(0, 0);

	samplerBuffer sbuf;
	pair = uvec2(sbuf);

	sampler1D s1d;
	pair = uvec2(s1d);

	sampler1DArray s1darr;
	pair = uvec2(s1darr);

	sampler2D s2d;
	pair = uvec2(s2d);

	sampler2DMS s2dms;
	pair = uvec2(s2dms);

	sampler2DArray s2darr;
	pair = uvec2(s2darr);

	sampler2DMSArray s2dmsarr;
	pair = uvec2(s2dmsarr);

	sampler2DRect s2drect;
	pair = uvec2(s2drect);

	sampler3D s3d;
	pair = uvec2(s3d);

	samplerCube scube;
	pair = uvec2(scube);

	samplerCubeArray scubearr;
	pair = uvec2(scubearr);

	sampler1DShadow s1ds;
	pair = uvec2(s1ds);

	sampler2DShadow s2ds;
	pair = uvec2(s2ds);

	samplerCubeShadow scubes;
	pair = uvec2(scubes);

	sampler2DRectShadow s2drects;
	pair = uvec2(s2drects);

	sampler1DArrayShadow s1darrs;
	pair = uvec2(s1darrs);

	sampler2DArrayShadow s2darrs;
	pair = uvec2(s2darrs);

	samplerCubeArrayShadow scubearrs;
	pair = uvec2(scubearrs);
}

void uvec2_to_any_sampler_type()
{
	uvec2 pair = uvec2(0, 0);

	samplerBuffer sbuf = samplerBuffer(pair);
	sampler1D s1d = sampler1D(pair);
	sampler1DArray s1darr = sampler1DArray(pair);
	sampler2D s2d = sampler2D(pair);
	sampler2DMS s2dms = sampler2DMS(pair);
	sampler2DArray s2darr = sampler2DArray(pair);
	sampler2DMSArray s2dmsarr = sampler2DMSArray(pair);
	sampler2DRect s2drect = sampler2DRect(pair);
	sampler3D s3d = sampler3D(pair);
	samplerCube scube = samplerCube(pair);
	samplerCubeArray scubearr = samplerCubeArray(pair);
	sampler1DShadow s1ds = sampler1DShadow(pair);
	sampler2DShadow s2ds = sampler2DShadow(pair);
	samplerCubeShadow scubes = samplerCubeShadow(pair);
	sampler2DRectShadow s2drects = sampler2DRectShadow(pair);
	sampler1DArrayShadow s1darrs = sampler1DArrayShadow(pair);
	sampler2DArrayShadow s2darrs = sampler2DArrayShadow(pair);
	samplerCubeArrayShadow scubearrs = samplerCubeArrayShadow(pair);
}

void main()
{
	any_sampler_type_to_uvec2();
	uvec2_to_any_sampler_type();
}
