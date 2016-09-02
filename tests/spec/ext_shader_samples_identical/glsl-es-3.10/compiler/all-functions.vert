// [config]
// expect_result: pass
// glsl_version: 3.10
// require_extensions: GL_EXT_shader_samples_identical GL_OES_texture_storage_multisample_2d_array
// [end config]

#version 310 es
#extension GL_EXT_shader_samples_identical: require
#extension GL_OES_texture_storage_multisample_2d_array: require

uniform mediump sampler2DMS s1;
uniform mediump isampler2DMS s2;
uniform mediump usampler2DMS s3;
uniform mediump sampler2DMSArray s4;
uniform mediump isampler2DMSArray s5;
uniform mediump usampler2DMSArray s6;

flat out ivec2 data;

void main()
{
	const ivec2 p2 = ivec2(10, 10);
	const ivec3 p3 = ivec3(15, 11, 0);

	data = ivec2(int(textureSamplesIdenticalEXT(s1, p2)) +
		     int(textureSamplesIdenticalEXT(s2, p2)) +
		     int(textureSamplesIdenticalEXT(s3, p2)),
		     int(textureSamplesIdenticalEXT(s4, p3)) +
		     int(textureSamplesIdenticalEXT(s5, p3)) +
		     int(textureSamplesIdenticalEXT(s6, p3)));

	gl_Position = vec4(0);
}
