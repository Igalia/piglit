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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Chris Forbes <chrisf@ijw.co.nz>
 *
 */

/*
 * fbo-mrt-alphatest asserts correct behavior for alpha-testing of fragments
 * when multiple color buffers are being rendered to. In particular, the alpha
 * component of the first color output is used for the alpha test.
 *
 * This is important for deferred renderers which use alpha-test, and is a
 * significant edge case for the i965 driver.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
GLuint fbo;
GLint prog;
GLuint color0, color1;

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(130);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &color0);
	glBindTexture(GL_TEXTURE_2D, color0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);

	glGenTextures(1, &color1);
	glBindTexture(GL_TEXTURE_2D, color1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);

	glDrawBuffers(2, buffers);

	prog = piglit_build_simple_program(
		"#version 130\n"
		"in vec4 pos;\n"
		"void main() {\n"
		"	gl_Position = pos;\n"
		"}\n",

		"#version 130\n"
		"void main() {\n"
		"	float alpha = float(int(gl_FragCoord.x / 16 + gl_FragCoord.y / 16) % 2);\n"
		"	gl_FragData[0] = vec4(1.0, 0.0, 0.0, alpha);\n"
		"	gl_FragData[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"}\n"
		);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Setup for test failed.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}


enum piglit_result
piglit_display(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0,0,1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	glAlphaFunc(GL_GEQUAL, 0.5f);
	glEnable(GL_ALPHA_TEST);

	glUseProgram(prog);
	glViewport(0, 0, 64, 64);
	piglit_draw_rect(-1, -1, 2, 2);

	glDisable(GL_ALPHA_TEST);

	/* visualize it */
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, 128, 64);
	glClearColor(0,0,0.5,0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, color0);
	piglit_draw_rect_tex(-1, -1, 1, 2,
			0, 0, 1, 1);
	glBindTexture(GL_TEXTURE_2D, color1);
	piglit_draw_rect_tex(0, -1, 1, 2,
			0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);

	{
		bool pass = true;
		float red[] = {1,0,0};
		float green[] = {0,1,0};
		float blue[] = {0,0,1};
		pass = piglit_probe_pixel_rgb(4, 4, blue) && pass;
		pass = piglit_probe_pixel_rgb(12, 4, red) && pass;
		pass = piglit_probe_pixel_rgb(64 + 4, 4, blue) && pass;
		pass = piglit_probe_pixel_rgb(64 + 12, 4, green) && pass;

		piglit_present_results();

		return pass ? PIGLIT_PASS : PIGLIT_FAIL;
	}
}
