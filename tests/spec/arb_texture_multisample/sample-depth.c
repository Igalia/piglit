/*
 * Copyright © 2014 Intel Corporation
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
 *
 * Authors: Chris Forbes
 */
#include "piglit-util-gl.h"

/* File: sample-depth.c
 *
 * Tests whether sampling from a multisample depth texture works correctly
 * after having rendered into it.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_SAMPLES 4
#define TEX_WIDTH 64
#define TEX_HEIGHT 64

GLuint prog, fbo;

float green[] = {0,1,0,0};

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	/* draw a quad with depth varying from 1.0 at the left side to -1.0 at
	 * the right side (NDC) */

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	glUseProgram(0);

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	glDisable(GL_DEPTH_TEST);

	/* sample depth and write color to the default framebuffer
	 * so we can look at it */

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, TEX_WIDTH, TEX_HEIGHT, green) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	piglit_require_extension("GL_ARB_texture_multisample");

	/* setup an fbo with multisample depth texture */

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
			NUM_SAMPLES, GL_DEPTH_COMPONENT24,
			TEX_WIDTH, TEX_HEIGHT,
			GL_TRUE);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D_MULTISAMPLE,
			tex,
			0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error during tex/fbo setup; no point continuing.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* shader to sample from it */

	prog = piglit_build_simple_program(
			"#version 130\n"
			"out vec4 ndc;\n"
			"void main() {\n"
			"	gl_Position = gl_Vertex;\n"
			"	ndc = gl_Vertex;\n"
			"}\n",

			"#version 130\n"
			"#extension GL_ARB_texture_multisample: require\n"
			"uniform sampler2DMS s;\n"
			"const int sample_id = 0;\n"
			"const float close_enough = 0.01f;\n"
			"in vec4 ndc;\n"
			"void main() {\n"
			"	vec4 res = texelFetch(s,\n"
			"		ivec2(gl_FragCoord.xy),\n"
			"		sample_id);\n"
			"	float expected = 0.5f * abs(ndc.y + ndc.x);\n"
			"	if (distance(expected, res.x) > close_enough) {\n"
			"		gl_FragColor = vec4(1,0,0,0);\n"
			"	} else {\n"
			"		gl_FragColor = vec4(0,1,0,0);\n"
			"	};\n"
			"}\n");
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "s"), 0);
	

	if (!prog || !piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error during shader setup; no point continuing.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}
