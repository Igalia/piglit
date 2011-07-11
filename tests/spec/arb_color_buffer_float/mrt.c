/*
 * Copyright © 2010 Luca Barbieri
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
 *
 */

/** @file spec/arb_color_buffer_float/mrt.c
 *
 * Tests that fragment color clamping affects MRT rendering as
 * specified by ARB_color_buffer_float and OpenGL 4.1
 *
 * Note that the specification is not fully clear here.
* It *seems* to mean that clamping does *not* depend on the target framebuffer type,
 * but rather the data type of the shader variable and whether there is *any* floating-point
 * buffer in case of GL_FIXED_ONLY clamping.
 *
 * On ATI Radeon HD 58xx, dishomogeneous framebuffers are incomplete.
 * TODO: what happens on GeForce 8xxx, GTX 2xx and GTX 4xx?
 */

/* If clamp is TRUE, fragment color
 * clamping is enabled; if clamp is FALSE, fragment color clamping is disabled. If
 * clamp is FIXED_ONLY, fragment color clamping is enabled if all enabled color
 * buffers have fixed-point components.
 *
 * If fragment color clamping is enabled
 * and the color buffer has an unsigned normalized fixed-point, signed normalized
 * fixed-point, or floating-point format, the final fragment color, fragment data, or
 * varying out variable values written by a fragment shader are clamped to the range
 * [0, 1]. Only user-defined varying out variables declared as a floating-point type are
 * clamped and may be converted. If fragment color clamping is disabled, or the color
 * buffer has an integer format, the final fragment color, fragment data, or varying out
 * variable values are not modified.
 */

#include "common.h"

const char *mrt_vp_string =
	"!!ARBvp1.0\n" "MOV result.position, vertex.position;\n"
	"MOV result.texcoord[0], {7, -2.75, -0.25, 0.75};\n"
	"MOV result.texcoord[1], {7, -2.75, -0.25, 0.75};\n" "END\n";

const char *mrt_fp_string =
	"!!ARBfp1.0\n" "OPTION ARB_draw_buffers;\n"
	"MOV result.color[0], fragment.texcoord[0];\n"
	"MOV result.color[1], fragment.texcoord[1];\n" "END\n";

unsigned mrt_vp;
unsigned mrt_fp;

GLboolean
test()
{
	GLboolean pass = GL_TRUE;
	unsigned frag_clamp;

	for (frag_clamp = sanity ? 1 : 0; frag_clamp < (sanity ? 2 : 3); ++frag_clamp)
	{
		GLboolean cpass = GL_TRUE;
		GLboolean opass;
		GLboolean clamped = clamp_enums[frag_clamp] == GL_TRUE || (clamp_enums[frag_clamp] == GL_FIXED_ONLY_ARB && fixed);
		float *expected, *expected1;

		printf("MRT rendering in %s mode with fragment clamp %s (expecting %sclamping)\n", mrt_mode_strings[mrt_mode], clamp_strings[frag_clamp], clamped ? "" : "no ");
		if (!sanity)
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, clamp_enums[frag_clamp]);

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, mrt_vp);
		glEnable(GL_VERTEX_PROGRAM_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, mrt_fp);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);

		glClearColor(0.5, 0.5, 0.5, 0.5);
		glClear(GL_COLOR_BUFFER_BIT);

		piglit_draw_rect(-1, -1, 1, 1);

		glDisable(GL_VERTEX_PROGRAM_ARB);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);

		expected = clamped ? clamped_pixels :
			   fixed0 ? clamped_pixels :
			   pixels;
		expected1 = clamped ? clamped_pixels :
			    fixed1 ? clamped_pixels :
			    pixels;

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		printf("Probing buffer 0 (%s)\n", fixed0 ? "fixed point" : "floating point");
		cpass = piglit_probe_pixel_rgba(0, 0, expected) && cpass;
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		printf("Probing buffer 1 (%s)\n", fixed1 ? "fixed point" : "floating point");
		cpass = piglit_probe_pixel_rgba(0, 0, expected1) && cpass;
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		error = glGetError();
		if(error)
		{
			printf("GL error after MRT 0x%04X\n", error);
			return GL_FALSE;
		}

		opass = cpass;
		if(!cpass && nvidia_driver && !fixed && clamped)
		{
			printf("nVidia driver ***MAJOR BUG***: they never clamp when using MRT on floating point targets!\n");
			opass = GL_TRUE;
		}
		pass = opass && pass;
	}
	return pass;
}

unsigned
init()
{
	GLint num;

	piglit_require_extension("GL_ARB_vertex_program");
	piglit_require_extension("GL_ARB_fragment_program");
	piglit_require_extension("GL_ARB_draw_buffers");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &num);
	if (num < 2) {
		printf("Test requires 2 draw buffers, found %d\n", num);
		piglit_report_result(PIGLIT_SKIP);
	}

	mrt_vp = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, mrt_vp_string);
	mrt_fp = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, mrt_fp_string);

	return TEST_MRT;
}
