// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: true
// [end config]
//
// Section 4.3.8.2 (Output Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "layout(triangle_strip, max_vertices = 60) out; // order does not matter
//     layout(max_vertices = 60) out;                 // redeclaration okay (*)
//     layout(triangle_strip) out;                    // redeclaration okay (*)
//     layout(points) out;                            // error, contradicts triangle_strip
//     layout(max_vertices = 30) out;                 // error, contradicts 60"
//
//    ...
//
//    "All geometry shader output layout declarations in a program must declare the
//     same layout and same value for max_vertices."
//
// This test verifies the case marked with (*), namely that no error
// results from declaring a geometry shader output layout that is
// consistent with a previously declared geometry shader output layout.

#version 150

layout(lines) in;
layout(line_strip, max_vertices=3) out;

in vec4 pos[];

layout(line_strip, max_vertices=3) out;
layout(max_vertices=3) out;
layout(line_strip) out;

void main()
{
}
