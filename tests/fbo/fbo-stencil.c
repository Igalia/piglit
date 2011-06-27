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

/** @file fbo-stencil.c
 *
 * Tests glClear, glReadPixels, glDrawPixels, and glCopyPixels
 * with stencil buffers.
 */

#include "piglit-util.h"

#define BUF_SIZE 123
int piglit_width = BUF_SIZE;
int piglit_height = BUF_SIZE;
int piglit_window_mode = GLUT_DOUBLE;
enum {
	CLEAR,
	READPIXELS,
	DRAWPIXELS,
	COPYPIXELS
};
int test = CLEAR;

#define F(name) #name, name

struct format {
	const char *name;
	GLuint iformat, format;
	const char *extension;
} formats[] = {
	{F(GL_STENCIL_INDEX1),    GL_STENCIL_INDEX, NULL},
	{F(GL_STENCIL_INDEX4),    GL_STENCIL_INDEX, NULL},
	{F(GL_STENCIL_INDEX8),    GL_STENCIL_INDEX, NULL},
	{F(GL_STENCIL_INDEX16),   GL_STENCIL_INDEX, NULL},

	{F(GL_DEPTH24_STENCIL8),  GL_DEPTH_STENCIL,
	 "GL_EXT_packed_depth_stencil"},

	{F(GL_DEPTH32F_STENCIL8), GL_DEPTH_STENCIL,
	 "GL_ARB_depth_buffer_float"}
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

	glClearStencil(0x34);
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x34, ~0);

	glColor3fv(green);
	piglit_draw_rect(-1, -1, 2, 2);
	glColor3f(1, 1, 1);

	glDisable(GL_STENCIL_TEST);

	res = piglit_probe_rect_rgb(0, 0, BUF_SIZE, BUF_SIZE, green) ? PIGLIT_PASS : PIGLIT_FAIL;

	/* Display the colorbuffer. */
	if (!piglit_automatic) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
		glBlitFramebufferEXT(0, 0, BUF_SIZE, BUF_SIZE, 0, 0, BUF_SIZE, BUF_SIZE,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glDeleteRenderbuffersEXT(1, &cb);

	return res;
}

static enum piglit_result compare_stencil(void)
{
	int x, y, failures = 0;
	GLubyte stencil[BUF_SIZE*BUF_SIZE];
	GLubyte expected;

	/* Read stencil. */
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_STENCIL_INDEX,
		     GL_UNSIGNED_BYTE, stencil);

	/* Compare results. */
	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			if (y < BUF_SIZE/2)
				expected = x < BUF_SIZE/2 ? 0x33 : 0x66;
			else
				expected = x < BUF_SIZE/2 ? 0x99 : 0xbb;

			if (stencil[y*BUF_SIZE+x] != expected) {
				failures++;
				if (failures < 20) {
					printf("Stencil at %i,%i   Expected: %02x   Observed: %02x\n",
						x, y, expected, stencil[y*BUF_SIZE+x]);
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

static enum piglit_result test_readpixels(void)
{
	/* Clear stencil to 0xfe. */
	glClearStencil(0xfe);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Initialize stencil. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glStencilFunc(GL_ALWAYS, 0x33, ~0);
	piglit_draw_rect(-1, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x66, ~0);
	piglit_draw_rect(0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x99, ~0);
	piglit_draw_rect(-1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbb, ~0);
	piglit_draw_rect(0, 0, 1, 1);

	glDisable(GL_STENCIL_TEST);

	return compare_stencil();
}

static enum piglit_result test_drawpixels(void)
{
	int x, y;
	GLubyte drawbuf[BUF_SIZE*BUF_SIZE];

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			if (y < BUF_SIZE/2)
				drawbuf[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? ~0x33 : ~0x66;
			else
				drawbuf[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? ~0x99 : ~0xbb;
		}
	}

	/* Clear stencil to 0xfe. */
	glClearStencil(0xfe);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Draw pixels. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, drawbuf);

	/* Invert bits. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
	piglit_draw_rect(-1, -1, 2, 2);
	glDisable(GL_STENCIL_TEST);

	return compare_stencil();
}

static enum piglit_result test_copypixels(void)
{
	/* Clear stencil to 0xfe. */
	glClearStencil(0xfe);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Initialize stencil. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	/* Set the upper-right corner to 0x33 and copy the content the lower-left one. */
	glStencilFunc(GL_ALWAYS, 0x33, ~0);
	piglit_draw_rect(0, 0, 1, 1);
	glCopyPixels(BUF_SIZE/2, BUF_SIZE/2, BUF_SIZE/2, BUF_SIZE/2, GL_STENCIL);

	/* Initialize the other corners. */
	glStencilFunc(GL_ALWAYS, 0x66, ~0);
	piglit_draw_rect(0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x99, ~0);
	piglit_draw_rect(-1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbb, ~0);
	piglit_draw_rect(0, 0, 1, 1);

	glDisable(GL_STENCIL_TEST);

	return compare_stencil();
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
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT,
				     GL_RENDERBUFFER_EXT, rb);
	glViewport(0, 0, BUF_SIZE, BUF_SIZE);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	switch (test) {
	case CLEAR:
		puts("Testing glClear(stencil).");
		res = test_clear();
		break;
	case READPIXELS:
		puts("Testing glReadPixels(stencil).");
		res = test_readpixels();
		break;
	case DRAWPIXELS:
		puts("Testing glDrawPixels(stencil).");
		res = test_drawpixels();
		break;
	case COPYPIXELS:
		puts("Testing glCopyPixels(stencil).");
		res = test_copypixels();
		break;
	default:
		assert(0);
	}

	/* Cleanup. */
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteRenderbuffersEXT(1, &rb);

	glutSwapBuffers();

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
