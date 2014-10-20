/*
 * Copyright © 2010 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Marek Olšák <maraeo@gmail.com>
 *
 */

/** @file arbfp.c
 *
 * Tests that with OPTION ATI_draw_buffers, fragment.color does not
 * override fragment.color[1].
 *
 * From the ATI_draw_buffers spec:
 *
 *     '"[n]" is optional -- color <n> is used if specified; color 0
 *      is used otherwise.'
 *
 * This bug in Mesa impacted Trine.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static float result0[] = {0.7, 0.6, 0.3, 0.1};
static float result1[] = {0.2, 0.9, 0.5, 0.8};

static const char *mrt_fp_string =
	"!!ARBfp1.0\n"
	"OPTION ATI_draw_buffers;\n"
	"MOV result.color[1], fragment.texcoord[1];\n"
	"MOV result.color, fragment.texcoord[0];\n"
	"END\n";

static GLuint
attach_texture(int i)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT + i,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex0, tex1, fb;
	GLenum status;
	const GLenum attachments[] = {
		GL_COLOR_ATTACHMENT0_EXT,
		GL_COLOR_ATTACHMENT1_EXT,
	};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	tex0 = attach_texture(0);
	tex1 = attach_texture(1);

	glDrawBuffersATI(2, attachments);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Clear render targets (textures) to red */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMultiTexCoord4fv(GL_TEXTURE0, result0);
	glMultiTexCoord4fv(GL_TEXTURE1, result1);

        glEnable(GL_FRAGMENT_PROGRAM_ARB);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	/* Draw the two green textures to halves of the window. */
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, tex0);
	piglit_draw_rect_tex(0, 0,
			     piglit_width / 2, piglit_height,
			     0, 0, 1, 1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	piglit_draw_rect_tex(piglit_width / 2, 0,
			     piglit_width / 2, piglit_height,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex0);
	glDeleteTextures(1, &tex1);
	glDeleteFramebuffersEXT(1, &fb);

	pass = pass && piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
					     result0);
	pass = pass && piglit_probe_rect_rgba(piglit_width / 2, 0, piglit_width / 2, piglit_height,
					     result1);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint num;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ATI_draw_buffers");
	piglit_require_extension("GL_ARB_fragment_program");
	piglit_require_extension("GL_ARB_vertex_program");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &num);
	if (num < 2)
		piglit_report_result(PIGLIT_SKIP);

	piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, mrt_fp_string);
}
