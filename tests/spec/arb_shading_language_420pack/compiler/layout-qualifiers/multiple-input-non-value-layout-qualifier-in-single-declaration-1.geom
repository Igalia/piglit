// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack GL_ARB_gpu_shader5
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration."
//
// Section 4.3.8.1 (Input Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "For inputs declared without an array size, including
//     intrinsically declared inputs (i.e., gl_in), a layout must be
//     declared before any use of the method length() or other array
//     use requiring its size be known."

#version 150
#extension GL_ARB_shading_language_420pack: enable
#extension GL_ARB_gpu_shader5 : enable

layout(lines) layout(invocations=4) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 Color1[];

uniform int foo[Color1.length() == 2 ? 1 : -1];

void main()
{
}
