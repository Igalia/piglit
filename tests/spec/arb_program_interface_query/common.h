/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__


/* /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\
 * If you modify any of these shaders, you need to modify the resources names in
 * resource-query.c.
 * /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\
 */

static const char vs_empty[] =
	"#version 150\n"
	"void main() {\n"
	"}";

static const char fs_empty[] =
	"#version 150\n"
	"void main() {\n"
	"}";

static const char vs_std[] =
	"#version 150\n"
	"struct vs_struct {\n"
	"	vec4 a[2];\n"
	"};\n"
	"uniform vs_uniform_block {\n"
	"	vec4 vs_test;\n"
	"};\n"
	"uniform vs_struct sa[2];\n"
	"in vec4 vs_input0;\n"
	"in vec4 vs_input1;\n"
	"void main() {\n"
	"	gl_Position = vs_input0 * vs_test * vs_input1 + sa[0].a[1] +"
	"	              sa[1].a[1];\n"
	"}";

const char gs_std[] =
	"#version 150\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 6) out;\n"
	"uniform gs_uniform_block {\n"
	"	vec4 gs_test;\n"
	"};\n"
	"in vec4 gs_input[3];\n"
	"out vec4 gs_output0;\n"
	"void main() {\n"
	"	for (int i = 0; i < 6; i++) {\n"
	"		gl_Position = gs_input[i % 3] *"
	"		              gl_in[i % 3].gl_Position * gs_test;\n"
	"		gs_output0 = gs_input[0];\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n";

static const char fs_std[] =
	"#version 150\n"
	"uniform fs_uniform_block {"
	"	vec4 fs_color;\n"
	"	float fs_array[4];\n"
	"};"
	"in vec4 fs_input1;\n"
	"out vec4 fs_output0;\n"
	"out vec4 fs_output1;\n"
	"void main() {\n"
		"fs_output0 = fs_color * fs_input1 * fs_array[2];\n"
		"fs_output1 = fs_color * fs_input1 * fs_array[3];\n"
	"}";

static const char vs_stor[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"buffer vs_buffer_block { vec4 vs_buf_var; };"
	"out vec4 vs_output1;\n"
	"void main() {\n"
		"vs_output1 = vs_buf_var;\n"
	"}";

static const char gs_stor[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 6) out;\n"
	"buffer gs_buffer_block { vec4 gs_buf_var; };"
	"in vec4 vs_output1[3];\n"
	"void main() {\n"
	"	for (int i = 0; i < 6; i++) {\n"
	"		gl_Position = vs_output1[i % 3] * gs_buf_var;\n"
	"		EmitVertex();\n"
	"	}\n"
	"}";

static const char fs_stor[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"buffer fs_buffer_block { vec4 fs_buf_var; };\n"
	"out vec4 fs_output0;\n"
	"void main() {\n"
	"	fs_output0 = fs_buf_var;\n"
	"}";

static const char vs_atom[] =
	"#version 150\n"
	"#extension GL_ARB_shader_atomic_counters : require\n"
	"layout (binding=0) uniform atomic_uint vs_counter;\n"
	"void main() {\n"
	"	atomicCounterIncrement(vs_counter);\n"
	"}";

static const char gs_atom[] =
	"#version 150\n"
	"#extension GL_ARB_shader_atomic_counters : require\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 6) out;\n"
	"layout (binding=1) uniform atomic_uint gs_counter;\n"
	"void main() {\n"
	"	atomicCounterIncrement(gs_counter);\n"
	"}";

static const char fs_atom[] =
	"#version 150\n"
	"#extension GL_ARB_shader_atomic_counters : require\n"
	"layout (binding=2) uniform atomic_uint fs_counter;\n"
	"void main() {\n"
	"	atomicCounterIncrement(fs_counter);\n"
	"}";

static const char vs_tfv[] =
	"#version 150\n"
	"in vec4 vs_input0;\n"
	"out vec4 vs_output1;\n"
	"out vec4 outValue;\n"
	"void main() {\n"
	"	vs_output1 = vs_input0;\n"
	"	outValue = vs_input0;\n"
	"}";

static const char vs_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"in vec4 vs_input0;\n"
	"subroutine vec4 vs_offset();\n"
	"subroutine uniform vs_offset VERTEX;\n"
	"subroutine (vs_offset) vec4 vss() { return vec4(1, 0, 0, 0); }\n"
	"subroutine (vs_offset) vec4 vss2() { return vec4(1, 0, 0, 0); }\n"
	"void main() {\n"
	"	gl_Position = vs_input0 + VERTEX();\n"
	"}";

static const char gs_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 6) out;\n"
	"subroutine vec4 gs_offset();\n"
	"subroutine uniform gs_offset GEOMETRY;\n"
	"subroutine (gs_offset) vec4 gss() { return vec4(1, 0, 0, 0); }\n"
	"in vec4 vs_output1[3];\n"
	"void main() {\n"
	"	for (int i = 0; i < 6; i++) {\n"
	"		gl_Position = vs_output1[i % 3] + GEOMETRY();\n"
	"		EmitVertex();\n"
	"	}\n"
	"}";

static const char fs_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"subroutine vec4 fs_offset();\n"
	"subroutine uniform fs_offset FRAGMENT;\n"
	"subroutine (fs_offset) vec4 fss() { return vec4(1, 0, 0, 1); }\n"
	"out vec4 fs_output0;\n"
	"void main() {\n"
	"	fs_output0 = FRAGMENT();\n"
	"}";

static const char tcs_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(vertices = 3) out;\n"
	"uniform tcs_uniform_block {\n"
	"	vec4 tcs_test;\n"
	"};\n"
	"out vec4 tcs_output[gl_MaxPatchVertices];\n"
	"in vec4 tcs_input[gl_MaxPatchVertices];\n"
	"patch out vec4 tcs_patch;\n"
	"subroutine vec4 tcs_offset();\n"
	"subroutine uniform tcs_offset TESS_CONTROL;\n"
	"subroutine (tcs_offset) vec4 tcss() { return vec4(1, 0, 0, 0); }\n"
	"void main() {\n"
	"	gl_out[gl_InvocationID].gl_Position = tcs_test +"
	"	                                      gl_in[0].gl_Position *"
	"	                                      TESS_CONTROL();\n"
	"	tcs_output[gl_InvocationID] = tcs_input[0] + TESS_CONTROL();\n"
	"}";

static const char tes_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(triangles) in;\n"
	"uniform tes_uniform_block {\n"
	"	vec4 tes_test;\n"
	"};\n"
	"out vec4 tes_output[1];\n"
	"in vec4 tes_input[gl_MaxPatchVertices];\n"
	"subroutine vec4 tes_offset();\n"
	"subroutine uniform tes_offset TESS_EVALUATION;\n"
	"subroutine (tes_offset) vec4 tess() { return vec4(1, 0, 0, 0); }\n"
	"void main() {\n"
	"	gl_Position = tes_test + gl_in[0].gl_Position +"
	"	              TESS_EVALUATION();\n"
	"	tes_output[0] = tes_input[0] + TESS_EVALUATION();\n"
	"}";

static const char cs_sub[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_shader_image_load_store : require\n"
	"#extension GL_ARB_compute_shader : require\n"
	"layout(local_size_x = 4) in;\n"
	"uniform cs_uniform_block {\n"
	"	uniform vec4 cs_test;\n"
	"};\n"
	"layout(size4x32) uniform image2D tex;\n"
	"subroutine vec4 com_offset();\n"
	"subroutine uniform com_offset COMPUTE;\n"
	"subroutine (com_offset) vec4 css() { return vec4(1, 0, 0, 0); }\n"
	"void main() {\n"
	"	imageStore(tex, ivec2(0.0), cs_test + COMPUTE());\n"
	"}";

#endif
