/*
 * Copyright © 2015 Intel Corporation
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

/** @file read-depth.c
 *
 * Tests NV_read_depth implementation
 *
 * Test iterates over table of depth buffer formats and expected types to
 * read values back from each format. For each format it renders a rectangle at
 * different depth levels, reads back a pixel and verifies expected depth value.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

const char *vs_source =
	"attribute vec4 vertex;\n"
	"uniform float depth;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(vertex.xy, depth, 1.0);\n"
	"}\n";

const char *fs_source =
	"void main()\n"
	"{\n"
	"}\n";

const GLenum tests[] = {
	GL_DEPTH_COMPONENT16, GL_UNSIGNED_INT_24_8_OES,
	GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT_24_8_OES,
	GL_DEPTH_COMPONENT32F, GL_FLOAT,
};

static bool
equals(float a, float b)
{
   return fabs(a - b) < 0.00001;
}

static GLuint
create_depth_fbo(GLenum depth_type)
{
	GLuint fbo, buffer;
	GLenum status;

	glGenRenderbuffers(1, &buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer);
	glRenderbufferStorage(GL_RENDERBUFFER,
		depth_type, piglit_width, piglit_height);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "error creating framebuffer, status 0x%x\n",
			status);
		return 0;
	}
	return fbo;
}

static bool
read_depth(GLenum type, float expect)
{
	GLfloat data;
	GLuint uint_pixel;
	if (type == GL_FLOAT) {
		glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, type,
			(void *) &data);
	} else {
		glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, type,
			(void *) &uint_pixel);
		uint_pixel = uint_pixel >> 8;
		data = (1.0 * ((float) uint_pixel)) / 16777215.0;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	if (!equals(data, expect)) {
		fprintf(stderr, "expected %f, got %f\n", expect, data);
		return false;
	}
	return true;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = true;
	const float step = 0.1;

	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Loop through formats listed in 'tests'. */
	for (unsigned j = 0; j < ARRAY_SIZE(tests); j += 2) {

		float expect = 0.0;

		GLuint fbo = create_depth_fbo(tests[j]);
		if (!fbo)
			return PIGLIT_FAIL;

		/* Step from -1.0 to 1.0, linear depth. Render a rectangle at
		 * depth i, read pixel and verify expected depth value.
		 */
		for (float i = -1.0; !equals(i, 1.0 + step); i += step) {

			glClear(GL_DEPTH_BUFFER_BIT);
			glUniform1f(glGetUniformLocation(prog, "depth"), i);

			piglit_draw_rect(-1, -1, 2, 2);

			if (!(read_depth(tests[j + 1], expect)))
				return PIGLIT_FAIL;

			expect += step / 2.0;
		}
		glDeleteFramebuffers(1, &fbo);
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_NV_read_depth");
	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);
}
