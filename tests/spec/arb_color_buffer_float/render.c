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

/** @file spec/arb_color_buffer_float/render.c
 *
 * Tests that vertex and fragment color clamping affects rendering as
 * specified by ARB_color_buffer_float.
 *
 * It also checks that fog, blending and logic op are done as specified by
 * ARB_color_buffer_float.
 *
 * Note that it's unclear what should happen when ARB_fog is specified in
 * the fragment program and fragment clamping is disabled: does the
 * color still get clamped before applying fog?
 *
 * Both ATI and nVidia do that, but they have fixed function fog in the tested
 * cards.
 * TODO: what happens on GeForce 8xxx, GTX 2xx and GTX 4xx?
 */

#include "common.h"

const char *blend_strings[] = { "disabled     ", "(ONE, ZERO)  ", "(CONST, ZERO)", "(ONE, ONE)   " };
GLenum blend_src[] = { 0, GL_ONE, GL_CONSTANT_COLOR, GL_ONE };
GLenum blend_dst[] = { 0, GL_ZERO, GL_ZERO, GL_ONE };

const char *vp_strings[] = {
	"!!ARBvp1.0\n"
	"MOV result.position, vertex.position;\n"
	"MOV result.fogcoord, 0;\n"
	"MOV result.color, {7, -2.75, -0.25, 0.75};\n" "END\n",

	"!!ARBvp1.0\n"
	"MOV result.position, vertex.position;\n"
	"MOV result.fogcoord, 0;\n"
	"MOV result.texcoord[0], {7, -2.75, -0.25, 0.75};\n" "END\n"
};

const char *fp_strings[] = {
	"!!ARBfp1.0\n"
	"MOV result.color, fragment.color;\n"
	"END\n",

	"!!ARBfp1.0\n"
	"MOV result.color, fragment.texcoord[0];\n"
	"END\n",

	"!!ARBfp1.0\n"
	"OPTION ARB_fog_linear;\n"
	"MOV result.color, fragment.color;\n"
	"END\n",

	"!!ARBfp1.0\n"
	"OPTION ARB_fog_linear;\n"
	"MOV result.color, fragment.texcoord[0];\n"
	"END\n",
};

unsigned vps[2];
unsigned fps[4];

GLboolean test()
{
	GLfloat probe[4];
	GLboolean pass = GL_TRUE;
	int npass = 0, total = 0;
	unsigned semantic, blend, logicop, vpmode, fpmode;
	unsigned vpmodes = 1 + !!GLEW_ARB_vertex_program;
	unsigned fpmodes = 1 + !!GLEW_ARB_fragment_program;
	unsigned vert_clamp, frag_clamp;

	glFogi(GL_FOG_MODE, GL_LINEAR);

	for (vert_clamp = 0; vert_clamp < (sanity ? 1 : 3); ++vert_clamp)
	for (frag_clamp = sanity ? 1 : 0; frag_clamp < (sanity ? 2 : 3); ++frag_clamp)
	for (semantic = 0; semantic < 2; ++semantic)
	for (blend = 0; blend < 4; ++blend)
	for (logicop = 0; logicop < 2; ++logicop)
	for (vpmode = 0; vpmode < vpmodes; ++vpmode)
	for (fpmode = 0; fpmode < fpmodes; ++fpmode)
	{
		char test_name[4096];
		unsigned clamped = (semantic == 0 && (clamp_enums[vert_clamp] == GL_TRUE || (clamp_enums[vert_clamp] == GL_FIXED_ONLY_ARB && fixed))) || clamp_enums[frag_clamp] == GL_TRUE || (clamp_enums[frag_clamp] == GL_FIXED_ONLY_ARB && fixed);
		float *expected;
		GLboolean cpass;
		GLboolean opass;

		if (!fpmode && semantic)
			continue;

		sprintf(test_name, "%s: Attrib %s  VertClamp %s  FragClamp %s  Blending %s  LogicOp %s  %s  %s  Fog %s (expecting %sclamping)",
                        format_name,
                        semantic ? "TEXCOORD0" : "COLOR    ",
                        clamp_strings[vert_clamp],
                        clamp_strings[frag_clamp],
                        blend_strings[blend],
                        logicop ? "Yes" : "No ",
                        vpmode ? "ARB_vp" : "ffvp  ",
                        fpmode ? "ARB_fp" : "fffp  ",
                        test_fog ? "Yes" : "No ",
                        clamped ? "" : "no ");
		if (!sanity) {
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, clamp_enums[vert_clamp]);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, clamp_enums[frag_clamp]);
		}

		glColor4f(0.1f, 0.2f, 0.3f, 0.4f);
		glTexCoord4f(0.5f, 0.6f, 0.7f, 0.8f);

		if (vpmode)
		{
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vps[semantic]);
			glEnable(GL_VERTEX_PROGRAM_ARB);
		}
		else
		{
			if (semantic == 0)
				glColor4f(pixels[0], pixels[1], pixels[2], pixels[3]);
			else
				glTexCoord4f(pixels[0], pixels[1], pixels[2], pixels[3]);
		}

		if (fpmode)
		{
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fps[semantic + (test_fog ? 2 : 0)]);
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
		}
		else
		{
			if (test_fog)
				glEnable(GL_FOG);
		}

		glClearColor(0.5, 0.5, 0.5, 0.5);
		glClear(GL_COLOR_BUFFER_BIT);

		if (blend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(blend_src[blend], blend_dst[blend]);
			glBlendColor(2.0f, 2.0f, 2.0f, 2.0f);
		}
		if (logicop)
			glEnable(GL_COLOR_LOGIC_OP);

		piglit_draw_rect(-1, -1, 1, 1);

		if (logicop)
			glDisable(GL_COLOR_LOGIC_OP);
		if (blend)
			glDisable(GL_BLEND);
		if (vpmode)
			glDisable(GL_VERTEX_PROGRAM_ARB);
		if (fpmode)
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
		else if (test_fog)
			glDisable(GL_FOG);

		if (blend == 2 && !logicop)
		{
			if (fixed_snorm)
				expected = clamped ? clamped_pixels : signed_clamped_pixels;
			else if (fixed)
				expected = clamped_pixels;
			else
				expected = clamped ? clamped_pixels_mul_2 : pixels_mul_2;
		}
		else if (blend == 3 && !logicop)
		{
			if (fixed_snorm)
				expected = clamped ? clamped_pixels_plus_half_signed_clamped : signed_clamped_pixels_plus_half_signed_clamped;
			else if (fixed)
				expected = clamped_pixels_plus_half_clamped;
			else
				expected = clamped ? clamped_pixels_plus_half : pixels_plus_half;
		}
		else
			expected = clamped ? clamped_pixels :
				   fixed_snorm ? signed_clamped_pixels :
				   fixed ? clamped_pixels :
				   pixels;

		opass = cpass = piglit_probe_pixel_rgba_silent(0, 0, expected, probe);

		if (nvidia_driver && clamped && !(semantic == 0 && clamp_enums[vert_clamp] == GL_TRUE) && clamp_enums[frag_clamp] == GL_TRUE && !fixed && fpmode && (!blend || logicop || format == GL_RGBA16F_ARB))
		{
			printf("nVidia driver known *** MAJOR BUG ***: they don't clamp fragment program results with ARB_fp on either fp32 with no blending or fp16!\n");
			opass = GL_TRUE;
		}
		if (nvidia_driver && clamped && !fixed && !fpmode && semantic == 0 && clamp_enums[vert_clamp] != GL_TRUE && clamp_enums[frag_clamp] == GL_TRUE)
		{
			printf("nVidia driver known *** MAJOR BUG ***: they don't clamp fragment program results with fffp, vertex clamp off and fragment clamp on fp16/fp32!\n");
			opass = GL_TRUE;
		}
		if (test_fog && fpmode)
		{
			//printf("Unclear specification on GL_ARB_fog_*\n");
			opass = GL_TRUE;
		}

		if (!opass) {
			printf("%s: %s\n", (cpass ? "PASS" : (opass ? "XFAIL" : "FAIL")), test_name);
			printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
			printf("  Observed: %f %f %f %f\n", probe[0], probe[1], probe[2], probe[3]);

		} else {
			npass++;
		}
		total++;

		pass = opass && pass;
	}

	printf("Summary: %i/%i passed.\n", npass, total);
	return pass;
}

unsigned
init()
{
	if (GLEW_ARB_vertex_program)
	{
		unsigned i;
		for (i = 0; i < 2; ++i)
			vps[i] = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, vp_strings[i]);
	}

	if (GLEW_ARB_fragment_program)
	{
		unsigned i;
		for (i = 0; i < 4; ++i)
			fps[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fp_strings[i]);
	}

	return TEST_SRT;
}
