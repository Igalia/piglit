// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_texture_multisample GL_EXT_shader_samples_identical
// [end config]

#version 150
#extension GL_ARB_texture_multisample: require
#extension GL_EXT_shader_samples_identical: require

uniform sampler2DMS s1;
uniform isampler2DMS s2;
uniform usampler2DMS s3;
uniform sampler2DMSArray s4;
uniform isampler2DMSArray s5;
uniform usampler2DMSArray s6;

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
	EmitVertex();
}
