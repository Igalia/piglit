/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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

/** @file fbo-depth.c
 *
 * Tests glClear, glReadPixels, glDrawPixels, glCopyPixels, glBlitFramebuffer
 * with depth buffers.
 */

#include "piglit-util-gl.h"

#define BUF_SIZE 123

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = BUF_SIZE;
	config.window_height = BUF_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum {
	CLEAR,
	READPIXELS,
	DRAWPIXELS,
	COPYPIXELS,
	BLIT
};
int test = CLEAR;

#define F(name) #name, name

struct format {
	const char *name;
	GLenum iformat;
	const char *extension;
} formats[] = {
	{F(GL_DEPTH_COMPONENT16), "GL_ARB_depth_texture"},
	{F(GL_DEPTH_COMPONENT24), "GL_ARB_depth_texture"},
	{F(GL_DEPTH_COMPONENT32), "GL_ARB_depth_texture"},
	{F(GL_DEPTH24_STENCIL8), "GL_EXT_packed_depth_stencil"},
	{F(GL_DEPTH_COMPONENT32F), "GL_ARB_depth_buffer_float"},
	{F(GL_DEPTH32F_STENCIL8), "GL_ARB_depth_buffer_float"}
};

struct format f;

static enum piglit_result test_clear(void)
{
	GLuint cb;
	GLenum status;
	float green[3] = {0, 1, 0};
	enum piglit_result res;

	/* Add a colorbuffer. */
	glGenRenderbuffersEXT(1, &cb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, cb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, BUF_SIZE, BUF_SIZE);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,
				     GL_RENDERBUFFER_EXT, cb);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_FAIL); /* RGBA8 must succeed. */
	}

	glClearDepth(0.75);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glColor3fv(green);
	glDepthFunc(GL_LEQUAL);
	piglit_draw_rect_z(0.499, -1, -1, 1, 2); /* 0.75 converted to clip space is 0.5. */
	glDepthFunc(GL_GEQUAL);
	piglit_draw_rect_z(0.501, 0, -1, 1, 2);
	glColor3f(1, 1, 1);

	glDisable(GL_DEPTH_TEST);

	res = piglit_probe_rect_rgb(0, 0, BUF_SIZE, BUF_SIZE, green) ? PIGLIT_PASS : PIGLIT_FAIL;

	/* Display the colorbuffer. */
	if (!piglit_automatic) {
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glBlitFramebufferEXT(0, 0, BUF_SIZE, BUF_SIZE, 0, 0, BUF_SIZE, BUF_SIZE,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glDeleteRenderbuffersEXT(1, &cb);

	return res;
}

static enum piglit_result compare()
{
	int x, y, failures = 0;
	GLfloat depth[BUF_SIZE*BUF_SIZE];
	GLfloat expected_depth;

	/* Read buffers. */
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_DEPTH_COMPONENT,
		     GL_FLOAT, depth);

	/* Compare results. */
	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {

			/* Skip the middle row and column of pixels because
			 * drawing polygons for the left/right and bottom/top
			 * quadrants may hit the middle pixels differently
			 * depending on minor transformation and rasterization
			 * differences.
			 */
			if (x == BUF_SIZE / 2 || y == BUF_SIZE / 2)
				continue;

			if (y < BUF_SIZE/2) {
				expected_depth = x < BUF_SIZE/2 ? 0.25 : 0.375;
			} else {
				expected_depth = x < BUF_SIZE/2 ? 0.625 : 0.75;
			}

			if (fabs(depth[y*BUF_SIZE+x] - expected_depth) > 0.001) {
				failures++;
				if (failures < 20) {
					printf("Depth at %i,%i   Expected: %f   Observed: %f\n",
						x, y, expected_depth, depth[y*BUF_SIZE+x]);
				} else if (failures == 20) {
					printf("...\n");
				}
			}
		}
	}
	if (failures)
		printf("Total failures: %i\n", failures);

	return failures == 0 ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result test_readpixels()
{
	/* Clear. */
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Initialize. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	piglit_draw_rect_z(-0.5, -1, -1, 1, 1);
	piglit_draw_rect_z(-0.25, 0, -1, 1, 1);
	piglit_draw_rect_z(0.25, -1, 0, 1, 1);
	piglit_draw_rect_z(0.5, 0, 0, 1, 1);

	glDisable(GL_DEPTH_TEST);

	return compare();
}

static enum piglit_result test_drawpixels()
{
	int x, y;
	GLfloat depth[BUF_SIZE*BUF_SIZE];

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			if (y < BUF_SIZE/2) {
				depth[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? 0.25 : 0.375;
			} else {
				depth[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? 0.625 : 0.75;
			}
		}
	}

	/* Clear. */
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Draw pixels. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
	glDisable(GL_DEPTH_TEST);

	return compare();
}

static enum piglit_result test_copy(void)
{
	/* Clear. */
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Initialize buffers. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	/* Set the upper-right corner to 0x3333 and copy the content to the lower-left one. */
	piglit_draw_rect_z(-0.5, 0, 0, 1, 1);
	if (test == BLIT)
		glBlitFramebufferEXT(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				     0, 0, BUF_SIZE/2, BUF_SIZE/2,
				     GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	else
		glCopyPixels(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE/2, BUF_SIZE/2, GL_DEPTH);

	/* Initialize the other corners. */
	piglit_draw_rect_z(-0.25, 0, -1, 1, 1);
	piglit_draw_rect_z(0.25, -1, 0, 1, 1);
	piglit_draw_rect_z(0.5, 0, 0, 1, 1);

	glDisable(GL_DEPTH_TEST);

	return compare();
}

enum piglit_result piglit_display(void)
{
	enum piglit_result res;
	GLuint fb, rb;
	GLenum status;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Create the FBO. */
	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, f.iformat, BUF_SIZE, BUF_SIZE);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,
				     GL_RENDERBUFFER_EXT, rb);
	glViewport(0, 0, BUF_SIZE, BUF_SIZE);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	switch (test) {
	case CLEAR:
		puts("Testing glClear(depth).");
		res = test_clear();
		break;
	case READPIXELS:
		puts("Testing glReadPixels(depth).");
		res = test_readpixels();
		break;
	case DRAWPIXELS:
		puts("Testing glDrawPixels(depth).");
		res = test_drawpixels();
		break;
	case COPYPIXELS:
	case BLIT:
		puts(test == BLIT ?
		     "Testing glBlitFramebuffer(depth)." :
		     "Testing glCopyPixels(depth).");
		res = test_copy();
		break;
	default:
		assert(0);
		res = PIGLIT_SKIP;
	}

	/* Cleanup. */
	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteRenderbuffersEXT(1, &rb);

	piglit_present_results();

	assert(glGetError() == 0);
	return res;
}

void piglit_init(int argc, char **argv)
{
	unsigned i, p;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");

	for (p = 1; p < argc; p++) {
		if (!strcmp(argv[p], "clear")) {
			test = CLEAR;
			continue;
		}
		if (!strcmp(argv[p], "readpixels")) {
			test = READPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "drawpixels")) {
			test = DRAWPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "copypixels")) {
			test = COPYPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "blit")) {
			test = BLIT;
			continue;
		}
		for (i = 0; i < sizeof(formats)/sizeof(*formats); i++) {
			if (!strcmp(argv[p], formats[i].name)) {
				if (formats[i].extension)
					piglit_require_extension(formats[i].extension);
				f = formats[i];
				printf("Testing %s.\n", f.name);
				break;
			}
		}
	}

	if (!f.name) {
		printf("Not enough parameters.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
