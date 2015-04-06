/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Red Hat
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

/** @file fbo-stencil8.c
 *
 * Tests glClear, glReadPixels, glBlitFramebuffer
 * with stencil buffers generated from stencil textures.
 */

#include "piglit-util-gl.h"

#define BUF_SIZE 123

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum {
	CLEAR,
	READPIXELS,
	BLIT
};
int test = CLEAR;

#define F(name) #name, name

struct format {
	const char *name;
	GLuint iformat;
	const char *extension;
} formats[] = {
	{F(GL_STENCIL_INDEX1),    NULL},
	{F(GL_STENCIL_INDEX4),    NULL},
	{F(GL_STENCIL_INDEX8),    NULL},
	{F(GL_STENCIL_INDEX16),   NULL},
};

struct format f;
GLuint mask;
GLuint vao;

static const char vs_text[] =
        "#version 130 \n"
        "in vec4 piglit_vertex;\n"
        "in vec4 piglit_texcoord;\n"
        "out vec4 colfs;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = piglit_vertex;\n"
        "   colfs = piglit_texcoord;\n"
        "}\n";

static const char fs_text[] =
        "#version 130 \n"
        "in vec4 colfs;\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = colfs;\n"
        "}\n";

static GLuint prog;

static enum piglit_result test_clear(void)
{
	GLuint cb_tex;
	GLenum status;
	float green[3] = {0, 1, 0};
	enum piglit_result res;

	glGenTextures(1, &cb_tex);
	glBindTexture(GL_TEXTURE_2D, cb_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, BUF_SIZE, BUF_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	/* Add a colorbuffer. */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cb_tex, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_FAIL); /* RGBA8 must succeed. */
	}

	glClearStencil(0x3456);
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x3456 & mask, ~0);

	glVertexAttrib3fv(PIGLIT_ATTRIB_TEX, green);
	piglit_draw_rect(-1, -1, 2, 2);
	glVertexAttrib3f(PIGLIT_ATTRIB_TEX, 1, 1, 1);

	glDisable(GL_STENCIL_TEST);

	res = piglit_probe_rect_rgb(0, 0, BUF_SIZE, BUF_SIZE, green) ? PIGLIT_PASS : PIGLIT_FAIL;

	/* Display the colorbuffer. */
	if (!piglit_automatic) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glBlitFramebufferEXT(0, 0, BUF_SIZE, BUF_SIZE, 0, 0, BUF_SIZE, BUF_SIZE,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glDeleteTextures(1, &cb_tex);

	return res;
}

static enum piglit_result compare_stencil(void)
{
	int x, y, failures = 0;
	GLushort stencil[BUF_SIZE*BUF_SIZE];
	GLushort expected;

	/* Read stencil. */
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_STENCIL_INDEX,
		     GL_UNSIGNED_SHORT, stencil);

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

			if (y < BUF_SIZE/2)
				expected = (x < BUF_SIZE/2 ? 0x3333 : 0x6666) & mask;
			else
				expected = (x < BUF_SIZE/2 ? 0x9999 : 0xbbbb) & mask;

			if (stencil[y*BUF_SIZE+x] != expected) {
				failures++;
				if (failures < 20) {
					printf("Stencil at %i,%i   Expected: 0x%02x   Observed: 0x%02x\n",
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
	glClearStencil(0xfefe);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Initialize stencil. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glStencilFunc(GL_ALWAYS, 0x3333 & mask, ~0);
	piglit_draw_rect(-1, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x6666 & mask, ~0);
	piglit_draw_rect(0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x9999 & mask, ~0);
	piglit_draw_rect(-1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbbbb & mask, ~0);
	piglit_draw_rect(0, 0, 1, 1);

	glDisable(GL_STENCIL_TEST);

	return compare_stencil();
}

static enum piglit_result test_copy(void)
{
	/* Clear stencil to 0xfe. */
	glClearStencil(0xfefe);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Initialize stencil. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	/* Set the upper-right corner to 0x3333 and copy the content to the lower-left one. */
	glStencilFunc(GL_ALWAYS, 0x3333 & mask, ~0);
	piglit_draw_rect(0, 0, 1, 1);
	glBlitFramebufferEXT(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
			     0, 0, BUF_SIZE/2, BUF_SIZE/2,
			     GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	/* Initialize the other corners. */
	glStencilFunc(GL_ALWAYS, 0x6666 & mask, ~0);
	piglit_draw_rect(0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x9999 & mask, ~0);
	piglit_draw_rect(-1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbbbb & mask, ~0);
	piglit_draw_rect(0, 0, 1, 1);

	glDisable(GL_STENCIL_TEST);

	return compare_stencil();
}

enum piglit_result piglit_display(void)
{
	enum piglit_result res;
	GLuint fb, tex;
	GLint stencil_size =8;
	GLenum status;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Create the FBO. */
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, BUF_SIZE, BUF_SIZE,
		     0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	glGenFramebuffersEXT(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT, tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
	glViewport(0, 0, BUF_SIZE, BUF_SIZE);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	mask = (1 << stencil_size) - 1;

	switch (test) {
	case CLEAR:
		puts("Testing glClear(stencil8).");
		res = test_clear();
		break;
	case READPIXELS:
		puts("Testing glReadPixels(stencil8).");
		res = test_readpixels();
		break;
	case BLIT:
		puts("Testing glBlitFramebuffer(stencil8).");
		res = test_copy();
		break;
	default:
		assert(0);
		res = PIGLIT_SKIP;
	}

	/* Cleanup. */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);

	piglit_present_results();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	return res;
}

void piglit_init(int argc, char **argv)
{
	unsigned i, p;
	GLuint fs, vs;
	piglit_require_extension("GL_ARB_texture_stencil8");

	for (p = 1; p < argc; p++) {
		if (!strcmp(argv[p], "clear")) {
			test = CLEAR;
			continue;
		}
		if (!strcmp(argv[p], "readpixels")) {
			test = READPIXELS;
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

        fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
        vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
        prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}
