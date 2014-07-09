/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2011 Vincent Lejeune
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file layout-std140.c
 *
 * This test checks the conformance of offset and size returned by
 * glGetActiveUniformsiv for uniform in a UBO whose layout is mandated
 * by the std140 layout qualifier
 *
 * The example shader and expected values for offset and size are
 * taken from the spec:
 *
 * http://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const struct result {
	const char *name;
	GLint offset;
	GLint size;
} expected_result[] = {
	{ "a", 0, 1 },
	{ "b", 8, 1 },
	{ "c", 16, 1 },
	{ "f.d", 32, 1},
	{ "f.e", 40, 1},
	{ "g", 48, 1},
	{ "h", 64, 2},
	{ "i", 96, 1},
	{ "o[0].j", 128, 1 },
	{ "o[0].k", 144, 1 },
	{ "o[0].l", 160, 2 },
	{ "o[0].m", 192, 1 },
	{ "o[0].n", 208, 2 },
	{ "o[1].j", 304, 1 },
	{ "o[1].k", 320, 1 },
	{ "o[1].l", 336, 2 },
	{ "o[1].m", 368, 1 },
	{ "o[1].n", 384, 2 },

	/* Section 2.11.4 (Uniform Variables), subsection Standard Uniform
	 * Block Layout, of the OpenGL 3.1 spec says (emphasis mine):
	 *
	 *     "(9) If the member is a structure, the base alignment of the
	 *     structure is <N>, where <N> is the largest base alignment value
	 *     of any of its members, and *rounded up to the base alignment of
	 *     a vec4*. The individual members of this sub-structure are then
	 *     assigned offsets by applying this set of rules recursively,
	 *     where the base offset of the first member of the sub-structure
	 *     is equal to the aligned offset of the structure. The structure
	 *     may have padding at the end; the base offset of the member
	 *     following the sub-structure is rounded up to the next multiple
	 *     of the base alignment of the structure."
	 */
	{ "s.s1.r", 0, 1 },
	{ "s.s2.g", 16, 1 },
	{ "s.s2.b", 20, 1 },
	{ "s.s2.a", 24, 1 },
};

static const char frag_shader_text[] =
	"#version 130\n"
	"#extension GL_ARB_uniform_buffer_object : enable \n"
	"\n"
	"struct f_struct {\n"
	"	int d;\n"
	"	bvec2 e;\n"
	"};\n"
	"\n"
	"struct o_struct {\n"
	"	uvec3 j;\n"
	"	vec2 k;\n"
	"	float l[2];\n"
	"	vec2 m;\n"
	"	mat3 n[2];\n"
	"};\n"
	"\n"
	"layout(std140) uniform test_ubo { \n"
	"	float a;\n"
	"	vec2 b;\n"
	"	vec3 c;\n"
	"	f_struct f;\n"
	"	float g;\n"
	"	float h[2];\n"
	"	mat2x3 i;\n"
	"	o_struct o[2];\n"
	"};\n"
	"\n"
	"struct S1 {\n"
	"	float r;\n"
	"};\n"
	"\n"
	"struct S2 {\n"
	"	float g;\n"
	"	float b;\n"
	"	float a;\n"
	"};\n"
	"\n"
	"struct S {\n"
	"       S1 s1;\n"
	"       S2 s2;\n"
	"};\n"
	"\n"
	"layout(std140) uniform ubo1 {\n"
	"	S s;\n"
	"};\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(a + b.x + c.x + float(f.d) + g + h[0] + i[0].x + o[1].k.x + s.s1.r + s.s2.g + s.s2.b + s.s2.a);\n"
	"}\n";

static void
init(void)
{
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(NULL, frag_shader_text);

	glUseProgram(prog);
}

static void
validate_offset_and_size()
{
	unsigned i, index;
	GLint offset, size;
	bool pass = true;

	printf("%8s%17s%10s%15s%8s\n",
	       "uniform",
	       "expected offset",
	       "offset",
	       "expected size",
	       "size");

	for(i = 0; i < ARRAY_SIZE(expected_result); i++) {
		glGetUniformIndices(prog, 1,
				    (const GLchar **)&expected_result[i].name,
				    &index);
		if (index == GL_INVALID_INDEX) {
			pass = false;
			printf("%8s%17d%10s%15d%8s INACTIVE\n",
			       expected_result[i].name,
			       expected_result[i].offset,
			       "",
			       expected_result[i].size,
			       "");
			continue;
		}

		glGetActiveUniformsiv(prog, 1, &index, GL_UNIFORM_OFFSET, &offset);
		glGetActiveUniformsiv(prog, 1, &index, GL_UNIFORM_SIZE, &size);

		if (offset != expected_result[i].offset ||
		    size != expected_result[i].size) {
			pass = false;
			printf("%8s%17d%10d%15d%8d FAIL\n",
			       expected_result[i].name,
			       expected_result[i].offset,
			       offset,
			       expected_result[i].size,
			       size);
		} else {
			printf("%8s%17d%10d%15d%8d PASS\n",
			       expected_result[i].name,
			       expected_result[i].offset,
			       offset,
			       expected_result[i].size,
			       size);
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	init();
	validate_offset_and_size();
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL;
}
