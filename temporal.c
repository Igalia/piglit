#version 150
#extension GL_ARB_gpu_shader_fp64 : require

uniform double tolerance;
uniform dvec4 expected;

flat in dvec4 out_to_fs;
out vec4 color;

void main()
{
	v = distance(result, expected) <= tolerance
		? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 1.0, 0.0, 1.0);

	dvec4 result = trunc(out_to_fs);
	color = distance(result, dvec4(expected)) <= tolerance
		? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}
