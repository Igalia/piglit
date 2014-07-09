/*
 * Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl.h"
#include "piglit-util-waffle.h"

#include "piglit_fbo_framework.h"
#include "piglit_wfl_framework.h"


static void
destroy(struct piglit_gl_framework *gl_fw)
{
	struct piglit_wfl_framework *wfl_fw = piglit_wfl_framework(gl_fw);

	if (wfl_fw == NULL)
		return;

	piglit_wfl_framework_teardown(wfl_fw);
	free(wfl_fw);
}

static void
run_test(struct piglit_gl_framework *gl_fw,
         int argc, char *argv[])
{
	enum piglit_result result = PIGLIT_PASS;

	if (gl_fw->test_config->init)
		gl_fw->test_config->init(argc, argv);
	if (gl_fw->test_config->display)
		result = gl_fw->test_config->display();
	gl_fw->destroy(gl_fw);
	piglit_report_result(result);
}

static bool
init_gl(struct piglit_wfl_framework *wfl_fw)
{
#ifdef PIGLIT_USE_OPENGL_ES1
	return false;
#else
	const struct piglit_gl_test_config *test_config = wfl_fw->gl_fw.test_config;

	GLuint tex, depth = 0;
	GLenum status;

#ifdef PIGLIT_USE_OPENGL
	if (piglit_get_gl_version() < 20)
		return false;

	if (!piglit_is_extension_supported("GL_ARB_framebuffer_object"))
		return false;
#endif

	glGenFramebuffers(1, &piglit_winsys_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       tex,
			       0);

	if (test_config->window_visual & (PIGLIT_GL_VISUAL_DEPTH |
					  PIGLIT_GL_VISUAL_STENCIL)) {
		/* Create a combined depth+stencil texture and attach it
		 * to the depth and stencil attachment points.
		 */
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL,
			     piglit_width, piglit_height, 0,
			     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_DEPTH_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_STENCIL_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"framebuffer status is incomplete, falling"
			"back to winsys\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteTextures(1, &depth);
		glDeleteTextures(1, &tex);
		return false;
	}

	return true;
#endif
}

struct piglit_gl_framework*
piglit_fbo_framework_create(const struct piglit_gl_test_config *test_config)
{
	struct piglit_wfl_framework *wfl_fw;
	struct piglit_gl_framework *gl_fw;

	int32_t platform;
	bool ok = true;

#ifdef PIGLIT_USE_OPENGL_ES1
	return NULL;
#endif

	platform = piglit_wfl_framework_choose_platform(test_config);

	if (test_config->window_samples > 1) {
		puts("The FBO mode doesn't support multisampling\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	wfl_fw = calloc(1, sizeof(*wfl_fw));
	gl_fw = &wfl_fw->gl_fw;

	ok = piglit_wfl_framework_init(wfl_fw, test_config, platform, NULL);
	if (!ok)
		goto fail;

	ok = init_gl(wfl_fw);
	if (!ok)
		goto fail;

	gl_fw->destroy = destroy;
	gl_fw->run_test = run_test;

	return gl_fw;

fail:
	destroy(gl_fw);
	return NULL;
}
