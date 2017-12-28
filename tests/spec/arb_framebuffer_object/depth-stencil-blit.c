/*
 * Copyright © 2016 Neha Bhende <bhenden@vmware.com>
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

/** @file depth-stencil-blit.c
 *
 * Tests glBlitFramebuffer with different draw and read depth/stencil buffers.
 */

#include "piglit-util-gl.h"

#define BUF_SIZE 241

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 14;

	config.window_width = BUF_SIZE;
	config.window_height = BUF_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

GLuint mask;
GLint stencil_size;
GLenum ds_format = GL_NONE;
bool depth = false, stencil = false;

static const struct {
	GLenum iformat;
	const char *extension;
} formats[] = {
	{GL_DEPTH_COMPONENT16, NULL},
	{GL_DEPTH_COMPONENT24, NULL},
	{GL_DEPTH_COMPONENT32, NULL},
	{GL_DEPTH24_STENCIL8, "GL_EXT_packed_depth_stencil"},
	{GL_DEPTH_COMPONENT32F, "GL_ARB_depth_buffer_float"},
	{GL_DEPTH32F_STENCIL8, "GL_ARB_depth_buffer_float"},
	{GL_STENCIL_INDEX1,    NULL},
	{GL_STENCIL_INDEX4,    NULL},
	{GL_STENCIL_INDEX8,    NULL},
	{GL_STENCIL_INDEX16,   NULL},
};


static bool
is_depth_stencil_format(GLenum format)
{
	switch (format) {
	case GL_DEPTH32F_STENCIL8:
	case GL_DEPTH24_STENCIL8:
		return true;
	default:
		return false;
	}
}


/** check if stencil buffer contains expected pattern */
static bool
compare_stencil(void)
{
	bool pass = true;

	pass = piglit_probe_rect_stencil(0, 0, BUF_SIZE/2, BUF_SIZE/2, 0x3333 & mask) && pass;
	pass = piglit_probe_rect_stencil(0, BUF_SIZE/2, BUF_SIZE/2, BUF_SIZE/2, 0xfefe & mask) && pass;
	pass = piglit_probe_rect_stencil(BUF_SIZE/2, 0, BUF_SIZE/2, BUF_SIZE, 0xfefe & mask) && pass;

	return pass;
}


/** check if depth buffer contains expected pattern */
static bool
compare_depth(void)
{
	bool pass = true;

	pass = piglit_probe_rect_depth(0, 0, BUF_SIZE/2, BUF_SIZE/2, 0.25) && pass;
	pass = piglit_probe_rect_depth(0, BUF_SIZE/2, BUF_SIZE/2, BUF_SIZE/2, 0.0) && pass;
	pass = piglit_probe_rect_depth(BUF_SIZE/2, 0, BUF_SIZE/2, BUF_SIZE, 0.0) && pass;

	return pass;
}


static enum piglit_result
test_stencil_blit(GLuint src_fbo, GLuint dst_fbo)
{
	bool pass = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo);

	/* Clear stencil to 0xfe. */
	glClearStencil(0xfefe & mask);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Initialize stencil. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	/* Set the upper-right corner to 0x3333 and copy the content to
	 * the lower-left one.
	 */
	glStencilFunc(GL_ALWAYS, 0x3333 & mask, ~0);
	piglit_draw_rect(0, 0, 1, 1);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);

	/*dst_fbo's depth and stencil*/
	glClearStencil(0xfefe & mask);
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (!depth & stencil)
		glBlitFramebuffer(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				  0, 0, BUF_SIZE/2, BUF_SIZE/2,
				  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	else
		glBlitFramebuffer(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				  0, 0, BUF_SIZE/2, BUF_SIZE/2,
				  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
				  GL_NEAREST);

	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo);

	/* make sure the depth buffer was not effected */
	if (is_depth_stencil_format(ds_format) && !depth) {
		pass = piglit_probe_rect_depth(0, 0, BUF_SIZE, BUF_SIZE, 0)
			&& pass;
	}

	pass = compare_stencil() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_depth_blit(GLuint src_fbo, GLuint dst_fbo)
{
	bool pass = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,src_fbo);

	/* Clear depth buffer to 0.0. */
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* set depth test state */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	/* Set the upper-right corner to 0x0 and copy the content to
	 * the lower-left one.
	 */
	piglit_draw_rect_z(-0.5, 0, 0, 1, 1);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);

	/*clear dst_fbo's depth and stencil*/
	glClearDepth(0);
	glClearStencil(0xfefe & mask);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (depth & !stencil)
		glBlitFramebuffer(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				  0, 0, BUF_SIZE/2, BUF_SIZE/2,
				  GL_DEPTH_BUFFER_BIT , GL_NEAREST);
	else
		// depth & stencil
		glBlitFramebuffer(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				  0, 0, BUF_SIZE/2, BUF_SIZE/2,
				  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
				  GL_NEAREST);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo);

	/* make sure the stencil buffer was not effected */
	if (is_depth_stencil_format(ds_format) && !stencil) {
		pass = piglit_probe_rect_stencil(0, 0, BUF_SIZE, BUF_SIZE,
						 0xfefe & mask) && pass;
	}

	pass = compare_depth() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


GLuint
create_fbo(void)
{
	GLuint fb, rb;
	GLenum status;

	/* Create the FBO. */
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, ds_format, BUF_SIZE, BUF_SIZE);

	if (stencil || is_depth_stencil_format(ds_format)) {
		glGetRenderbufferParameteriv(GL_RENDERBUFFER,
					     GL_RENDERBUFFER_STENCIL_SIZE,
					     &stencil_size);
		mask = (1 << stencil_size) - 1;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	if (depth || is_depth_stencil_format(ds_format))
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					  GL_RENDERBUFFER, rb);
	if (stencil || is_depth_stencil_format(ds_format))
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, rb);

	glViewport(0, 0, BUF_SIZE, BUF_SIZE);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	return fb;
}


enum piglit_result
piglit_display(void)
{
	enum piglit_result res;
	GLuint src_fbo, dst_fbo;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	src_fbo = create_fbo();
	dst_fbo = create_fbo();

	if (depth & !stencil)
		res = test_depth_blit(src_fbo, dst_fbo);
	else if (stencil & !depth)
		res = test_stencil_blit(src_fbo, dst_fbo);
	else {
		// depth & stencil
		res = test_depth_blit(src_fbo, dst_fbo);
		if (test_stencil_blit(src_fbo, dst_fbo) == PIGLIT_FAIL)
			res = PIGLIT_FAIL;
	}

	/* Cleanup. */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &src_fbo);
	glDeleteFramebuffers(1, &dst_fbo);

	piglit_present_results();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		res = PIGLIT_FAIL;
	return res;
}


void
print_usage(const char *piglit_test_name)
{
	printf("Not enough parameters or format is not supported by test.\n");
	printf("Usage: %s <format_type> <format> \n"
		"  where <format_type> : stencil/depth/depth_stencil \n"
		"  where <format> : \n"
		"     GL_DEPTH_COMPONENT16 \n"
		"     GL_DEPTH_COMPONENT24 \n"
		"     GL_DEPTH_COMPONENT32 \n"
		"     GL_DEPTH_COMPONENT32F \n"
		"     GL_STENCIL_INDEX1 \n"
		"     GL_STENCIL_INDEX4 \n"
		"     GL_STENCIL_INDEX8 \n"
		"     GL_STENCIL_INDEX16 \n"
		"     GL_DEPTH24_STENCIL8 \n"
		"     GL_DEPTH32F_STENCIL8 \n", piglit_test_name);
}


void
piglit_init(int argc, char **argv)
{
	bool skip = false;

	piglit_require_extension("GL_ARB_framebuffer_object");

	if (argc < 3) {
		print_usage(argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!strcmp(argv[1], "stencil"))
		stencil = true;
	else if (!strcmp(argv[1], "depth"))
		depth = true;
	else if (!strcmp(argv[1], "depth_stencil")) {
		depth = true;
		stencil = true;
	}
	else
		skip = true;

	const GLenum arg = piglit_get_gl_enum_from_name(argv[2]);
	for (int i = 0; i < ARRAY_SIZE(formats); i++) {
		if (arg == formats[i].iformat) {

			if (formats[i].extension &&
			    !piglit_is_extension_supported(formats[i].extension))
				continue;

			ds_format = formats[i].iformat;
			printf("Testing %s.\n",
			       piglit_get_gl_enum_name(ds_format));
			break;
		}
	}

	if (ds_format == GL_NONE)
		skip = true;

	if (skip) {
		print_usage(argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}
}
