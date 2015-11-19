// [config]
// expect_result: pass
// glsl_version: 1.10
// require_extensions: GL_ARB_texture_multisample GL_EXT_shader_samples_identical
// [end config]

#extension GL_ARB_texture_multisample: require
#extension GL_EXT_shader_samples_identical: require

uniform sampler2DMS s1;
uniform sampler2DMSArray s4;

void main()
{
	const ivec2 p2 = ivec2(10, 10);
	const ivec3 p3 = ivec3(15, 11, 0);

	gl_Position = vec4(float(textureSamplesIdenticalEXT(s1, p2)),
			   float(textureSamplesIdenticalEXT(s4, p3)),
			   0.0, 1.0);
}
