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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** \file blit-multiple-render-targets.cpp
 *
 *  This test verifies that glBlitFramebuffer() works as expected in case of
 *  multiple render targets.
 *
 *  From Section 4.3.2, page 268 of OpenGL 4.0 spec:
 *
 *     "When the color buffer is transferred, values are taken from the read
 *      buffer of the read framebuffer and written to each of the draw buffers
 *      of the draw framebuffer."
 **/

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static Fbo multisample_fbo;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig Config(num_samples, piglit_width, piglit_height);

	/* Setup fbo with renderbuffer and texture attachments */
	Config.num_rb_attachments = 2;
	Config.rb_attachment[0] = GL_COLOR_ATTACHMENT0;
	Config.rb_attachment[1] = GL_COLOR_ATTACHMENT1;

	Config.num_tex_attachments = 4;
	for (int i = 0; i < Config.num_tex_attachments; i++)
		Config.tex_attachment[i] = GL_COLOR_ATTACHMENT2 + i;

	multisample_fbo.setup(Config);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass = true;
	const GLenum bufs[] =
		{GL_COLOR_ATTACHMENT1,
		 GL_COLOR_ATTACHMENT4,
		 GL_COLOR_ATTACHMENT5,

		 GL_COLOR_ATTACHMENT0,
		 GL_COLOR_ATTACHMENT2,
		 GL_COLOR_ATTACHMENT3};

	const float expected[][4] =
		{{0.0, 1.0, 0.0, 1.0}, /* GL_COLOR_ATTACHMENT1 */
		 {0.0, 1.0, 0.0, 1.0}, /* GL_COLOR_ATTACHMENT4 */
		 {0.0, 1.0, 0.0, 1.0}, /* GL_COLOR_ATTACHMENT5 */

		 {0.0, 0.0, 1.0, 1.0}, /* GL_COLOR_ATTACHMENT0 */
		 {0.0, 0.0, 1.0, 1.0}, /* GL_COLOR_ATTACHMENT2 */
		 {0.0, 0.0, 1.0, 1.0}};/* GL_COLOR_ATTACHMENT3 */

	/* Clear piglit_winsys_fbo to green color */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear all color attachements of multisample_fbo to blue color */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_fbo.handle);
	glDrawBuffers(6, bufs);
	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set the draw buffers for BlitFramebuffer */
	glDrawBuffers(3, bufs);

	/* Blit from piglit_winsys_fbo to multisample_fbo. Blitting should
	 * happen in to all the draw buffers set above.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_fbo.handle);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT,
			  GL_NEAREST);

	for (unsigned i = 0; i < ARRAY_SIZE(bufs); i++) {
		bool result =true;

		/* Resolve the contents of multisample_fbo in to
		 * piglit_winsys_fbo.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisample_fbo.handle);
		glReadBuffer(bufs[i]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
				  0, 0, piglit_width, piglit_height,
				  GL_COLOR_BUFFER_BIT,
				  GL_NEAREST);

		/* Verify the contents */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		result = piglit_probe_rect_rgba(0, 0, piglit_width,
					      piglit_height, expected[i]);
		pass = result && pass;
		printf("Attachment = GL_COLOR_ATTACHMENT%d, Result = %s\n",
		       bufs[i] - GL_COLOR_ATTACHMENT0,
		       result ? "pass" :  "fail");
		piglit_present_results();
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
