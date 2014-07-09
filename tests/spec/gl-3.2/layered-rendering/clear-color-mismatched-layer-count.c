/*
 * Copyright © 2013 Intel Corporation
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

/** \file
 *
 * Test that a layered color clear clears all layers of all
 * framebuffer attachments, even if not all framebuffer attachments
 * have the same layer count.
 *
 * The test operates as follows:
 *
 * - Two textures are created, each with a different layer count.
 *
 * - Every layer of both textures is individually cleared to red.
 *
 * - Every layer of both textures is checked to verify that it has
 *   been properly cleared to red.
 *
 * - Both textures are bound to a single framebuffer in layered
 *   fashion, and then the entire framebuffer is cleared to green all
 *   at once.
 *
 * - Every layer of both textures is checked to verify that it has
 *   been cleared to green.
 */

#include "piglit-util-gl.h"
#include "piglit-util.h"

#define TEX_SIZE 128

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static void
check_completeness(const char *when, GLenum target)
{
	GLenum fbstatus = glCheckFramebufferStatus(target);
	if (fbstatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete when %s: %s\n", when,
		       piglit_get_gl_enum_name(fbstatus));
		piglit_report_result(PIGLIT_FAIL);
	}
}


static bool
check_layers(GLuint fbo, const GLuint *tex, int which_tex, bool expect_red,
	     int num_layers)
{
	const GLfloat red[] = { 1, 0, 0, 1 };
	const GLfloat green[] = { 0, 1, 0, 1 };
	bool pass = true;
	int layer;

	const GLfloat *expected_color;
	if (expect_red)
		expected_color = red;
	else
		expected_color = green;

	for (layer = 0; layer < num_layers; layer++) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glFramebufferTextureLayer(GL_READ_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0,
					  tex[which_tex], 0, layer);
		check_completeness("reading layers",
				   GL_READ_FRAMEBUFFER);
		printf("Probing texture %d, layer %d\n", which_tex, layer);

		pass = piglit_probe_rect_rgba(0, 0, TEX_SIZE,
					      TEX_SIZE,
					      expected_color) && pass;
	}
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	int layer, i;
	bool pass = true;
	GLuint tex[2], fbo;
	int num_layers[2] = { 4, 8 };
	GLenum draw_buffers[2] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1
	};

	glGenTextures(2, tex);
	for (i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D_ARRAY, tex[i]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
			     TEX_SIZE, TEX_SIZE,
			     num_layers[i], 0 /* border */,
			     GL_RGBA, GL_FLOAT, NULL);
	}
	glGenFramebuffers(1, &fbo);

	/* Bind each layer of the texture individually and clear it to red. */
	printf("Clearing each layer individually\n");
	glViewport(0, 0, TEX_SIZE, TEX_SIZE);
	glClearColor(1, 0, 0, 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	for (i = 0; i < 2; i++) {
		for (layer = 0; layer < num_layers[i]; layer++) {
			glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER,
						  GL_COLOR_ATTACHMENT0,
						  tex[i], 0, layer);
			check_completeness("clearing individual layers",
					   GL_DRAW_FRAMEBUFFER);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	/* Check that each layer of both textures is cleared to
	 * red. */
	for (i = 0; i < 2; i++) {
		pass = check_layers(fbo, tex, i, true, num_layers[i]) && pass;
	}

	/* Bind both textures to a single framebuffer in layered
	 * fashion, and clear the entire framebuffer to green.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	printf("Clearing all layers of both textures at once\n");
	glClearColor(0, 1, 0, 1);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			     tex[0], 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
			     tex[1], 0);
	check_completeness("clearing whole texture",
			   GL_DRAW_FRAMEBUFFER);
	glDrawBuffers(2, draw_buffers);
	glClear(GL_COLOR_BUFFER_BIT);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
	glDrawBuffers(1, draw_buffers);

	/* Check that each layer of both textures is cleared to the
	 * proper color.
	 */
	for (i = 0; i < 2; i++) {
		pass = check_layers(fbo, tex, i, false, num_layers[i]) && pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
