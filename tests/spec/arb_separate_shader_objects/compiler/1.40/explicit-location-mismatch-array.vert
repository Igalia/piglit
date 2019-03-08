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

in vec4 piglit_vertex;

layout(location = 0) out vec4 out1[2];
layout(location = 3) out vec4 out2;

void main()
{
	gl_Position = piglit_vertex;
	out1[0] = vec4(1.0, 0.0, 0.0, 1.0);
	out1[1] = vec4(1.0, 1.0, 0.0, 1.0);
	out2 = vec4(0.0);
}
