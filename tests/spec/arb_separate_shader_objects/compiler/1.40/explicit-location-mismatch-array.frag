// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects
// check_link: true
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.5 spec:
//
//    "For the purposes of determining if a non-vertex input matches an output
//    from a previous shader stage, the location layout qualifier (if any)
//    must match."
#version 140
#extension GL_ARB_separate_shader_objects : require

uniform int i;

layout(location = 1) in vec4 out1[2];
layout(location = 3) in vec4 out2;

out vec4 color;

void main()
{
	if (i == 0)
		color = out1[0];
	else if (i == 1)
		color = out1[1];
	else
		color = out2;
}
