/*
 * Copyright © 2011 Henri Verbeet <hverbeet@gmail.com>
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

/** @file fbo-depthtex.c
 *
 * Tests that texturing from a depth texture works after it was updated
 * through an FBO. Specifically test the case where the sampler view for the
 * depth texture would be created before the draw to that texture.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 640;
	config.window_height = 480;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static void
check_fbo_status()
{
	GLint status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}
}

enum piglit_result piglit_display(void)
{
	static const char *frag_src =
		"!!ARBfp1.0\n"
		"TEX result.color, fragment.texcoord[0], texture[0], 2D;\n"
		"END";

	static const struct
	{
		int x;
		int y;
		float color[3];
	}
	expected[] =
	{
		{  64, 240, {0.1f, 0.1f, 0.1f}},
		{ 192, 240, {0.3f, 0.3f, 0.3f}},
		{ 320, 240, {0.5f, 0.5f, 0.5f}},
		{ 448, 240, {0.7f, 0.7f, 0.7f}},
		{ 576, 240, {0.9f, 0.9f, 0.9f}},
	};

	GLuint fbo, frag, db_tex, cb_tex;
	GLboolean pass = GL_TRUE;
	char *tex_data;
	unsigned int i;

	frag = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, frag_src);

	glGenFramebuffersEXT(1, &fbo);
	glGenTextures(1, &cb_tex);
	glGenTextures(1, &db_tex);

	tex_data = calloc(piglit_width * piglit_height, 4);

	glBindTexture(GL_TEXTURE_2D, db_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, piglit_width, piglit_height,
		     0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, cb_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, piglit_width, piglit_height,
		     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, tex_data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	free(tex_data);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, db_tex, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cb_tex, 0);
	check_fbo_status();

	glViewport(0, 0, piglit_width, piglit_height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	/* Draw with the texture to make sure a sampler view is created for
	 * it before it's used as depth buffer by the FBO. */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glBindTexture(GL_TEXTURE_2D, db_tex);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, frag);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_TEXTURE_2D);
	piglit_draw_rect_tex(-1.0f, -1.0f, 2.0f, 2.0f,
			      0.0f,  0.0f, 1.0f, 1.0f);

	/* Fill the depth buffer with a gradient. */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	check_fbo_status();
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDisable(GL_TEXTURE_2D);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glVertex3f( 1.0f, -1.0f,  1.0f);
	glVertex3f( 1.0f,  1.0f,  1.0f);
	glEnd();

	/* Draw the depth texture as greyscale to the backbuffer. */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_TEXTURE_2D);
	piglit_draw_rect_tex(-1.0f, -1.0f, 2.0f, 2.0f,
			      0.0f,  0.0f, 1.0f, 1.0f);

	for (i = 0; i < sizeof(expected) / sizeof(*expected); ++i)
	{
		pass &= piglit_probe_pixel_rgb(expected[i].x, expected[i].y, expected[i].color);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_fragment_program");
}
