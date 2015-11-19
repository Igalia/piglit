// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_EXT_shader_samples_identical
// [end config]

#version 150
#extension GL_EXT_shader_samples_identical: require

uniform sampler2DMS s1;
uniform isampler2DMS s2;
uniform usampler2DMS s3;
uniform sampler2DMSArray s4;
uniform isampler2DMSArray s5;
uniform usampler2DMSArray s6;

out vec2 data;

void main()
{
	const ivec2 p2 = ivec2(10, 10);
	const ivec3 p3 = ivec3(15, 11, 0);

	data = vec2(float(textureSamplesIdenticalEXT(s1, p2)) +
		    float(textureSamplesIdenticalEXT(s2, p2)) +
		    float(textureSamplesIdenticalEXT(s3, p2)),
		    float(textureSamplesIdenticalEXT(s4, p3)) +
		    float(textureSamplesIdenticalEXT(s5, p3)) +
		    float(textureSamplesIdenticalEXT(s6, p3)));
}
