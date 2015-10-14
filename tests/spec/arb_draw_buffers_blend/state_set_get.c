/*
 * Copyright 2015 VMware, Inc.
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

/**
 * Test setting/getting state related to GL_ARB_draw_buffers_blend.
 * In particular, make sure glBlendFunc and glBlendEquation updates
 * all buffer state.
 *
 * Brian Paul
 * October 2015
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
PIGLIT_GL_TEST_CONFIG_END

static GLint num_buffers;

struct blend_state
{
	GLenum srcRGB, srcA, dstRGB, dstA;
	GLenum eqRGB, eqA;
};

#define MAX_BUFFERS 16

static struct blend_state state[MAX_BUFFERS];

static bool test_dlist = false;


static void
set_state(int buffer,
	  GLenum srcRGB, GLenum srcA,
	  GLenum dstRGB, GLenum dstA,
	  GLenum eqRGB, GLenum eqA)
{
	GLuint list = 0;

	state[buffer].srcRGB = srcRGB;
	state[buffer].srcA = srcA;
	state[buffer].dstRGB = dstRGB;
	state[buffer].dstA = dstA;
	state[buffer].eqRGB = eqRGB;
	state[buffer].eqA = eqA;

	if (test_dlist) {
		list = glGenLists(1);
		glNewList(list, GL_COMPILE);
	}

	if (srcRGB == srcA && dstRGB == dstA) {
		glBlendFunci(buffer, srcRGB, dstRGB);
	}
	else {
		glBlendFuncSeparateiARB(buffer, srcRGB, dstRGB, srcA, dstA);
	}
	if (eqRGB == eqA) {
		glBlendEquationiARB(buffer, eqRGB);
	}
	else {
		glBlendEquationSeparateiARB(buffer, eqRGB, eqA);
	}

	if (test_dlist) {
		glEndList(list);
		glCallList(list);
		glDeleteLists(list, 1);
	}

	piglit_check_gl_error(GL_NO_ERROR);
}


static void
set_state_all_buffers(GLenum srcRGB, GLenum srcA,
		      GLenum dstRGB, GLenum dstA,
		      GLenum eqRGB, GLenum eqA)
{
	GLuint list = 0;
	int i;

	for (i = 0; i < num_buffers; i++) {
		state[i].srcRGB = srcRGB;
		state[i].srcA = srcA;
		state[i].dstRGB = dstRGB;
		state[i].dstA = dstA;
		state[i].eqRGB = eqRGB;
		state[i].eqA = eqA;
	}

	if (test_dlist) {
		list = glGenLists(1);
		glNewList(list, GL_COMPILE);
	}

	if (srcRGB == srcA && dstRGB == dstA) {
		glBlendFunc(srcRGB, dstRGB);
	}
	else {
		glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
	}
	if (eqRGB == eqA) {
		glBlendEquation(eqRGB);
	}
	else {
		glBlendEquationSeparate(eqRGB, eqA);
	}

	if (test_dlist) {
		glEndList(list);
		glCallList(list);
		glDeleteLists(list, 1);
	}

	piglit_check_gl_error(GL_NO_ERROR);
}


static bool
check_state(int buffer)
{
	GLint srcRGB, srcA;
	GLint dstRGB, dstA;
	GLint eqRGB, eqA;

	glGetIntegeri_v(GL_BLEND_SRC_RGB, buffer, &srcRGB);
	glGetIntegeri_v(GL_BLEND_DST_RGB, buffer, &dstRGB);
	glGetIntegeri_v(GL_BLEND_SRC_ALPHA, buffer, &srcA);
	glGetIntegeri_v(GL_BLEND_DST_ALPHA, buffer, &dstA);
	glGetIntegeri_v(GL_BLEND_EQUATION_RGB, buffer, &eqRGB);
	glGetIntegeri_v(GL_BLEND_EQUATION_ALPHA, buffer, &eqA);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Unexpected GL error.\n");
		return false;
	}

	if (srcRGB != state[buffer].srcRGB ||
	    dstRGB != state[buffer].dstRGB ||
	    srcA != state[buffer].srcA ||
	    dstA != state[buffer].dstA ||
	    eqRGB != state[buffer].eqRGB ||
	    eqA != state[buffer].eqA) {
		printf("State doesn't match for buffer %d\n", buffer);
		return false;
	}

	return true;
}


static bool
check_state_all_buffers(void)
{
	int i;

	for (i = 0; i < num_buffers; i++) {
		if (!check_state(i)) {
			return false;
		}
	}

	return true;
}


static void
fail_msg(const char *msg)
{
	printf("Failure: %s (dlist mode = %d)\n", msg, test_dlist);
}


static bool
test_modes(void)
{
	bool pass = true;

	/* Initial setup and check (src/dst RGB==A, RGBeq==Aeq) */
	set_state_all_buffers(GL_ONE, GL_ONE,
			      GL_ZERO, GL_ZERO,
			      GL_FUNC_ADD, GL_FUNC_ADD);
	if (!check_state_all_buffers()) {
		fail_msg("Initial state check failed.\n");
		pass = false;
	}

	/* set one buffer's state */
	set_state(1,
		  GL_SRC_ALPHA, GL_ONE,
		  GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
		  GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT);
	if (!check_state_all_buffers()) {
		fail_msg("Setting one buffer state failed.\n");
		pass = false;
	}

	/* set all buffer state again */
	set_state_all_buffers(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
			      GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			      GL_FUNC_ADD, GL_FUNC_ADD);
	if (!check_state_all_buffers()) {
		fail_msg("Resetting all buffer state failed.\n");
		pass = false;
	}

	/* set last buffer's state (src/dst RGB==A, RGBeq==Aeq)*/
	set_state(num_buffers - 1,
		  GL_SRC_ALPHA, GL_SRC_ALPHA,
		  GL_ONE, GL_ONE,
		  GL_FUNC_SUBTRACT, GL_FUNC_SUBTRACT);
	if (!check_state_all_buffers()) {
		fail_msg("Setting last buffer state failed.\n");
		pass = false;
	}

	/* set first buffer's state */
	set_state(0,
		  GL_ONE, GL_ZERO,
		  GL_ZERO, GL_ONE,
		  GL_FUNC_SUBTRACT, GL_FUNC_ADD);
	if (!check_state_all_buffers()) {
		fail_msg("Setting first buffer state failed.\n");
		pass = false;
	}

	return pass;
}



enum piglit_result
piglit_display(void)
{
	/* never get here */
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	bool pass;

	piglit_require_extension("GL_ARB_draw_buffers_blend");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &num_buffers);

	num_buffers = MIN2(num_buffers, MAX_BUFFERS);

	if (num_buffers < 2) {
		printf("Need at least two draw buffers.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	printf("Testing %d buffers\n", num_buffers);

	test_dlist = false;
	pass = test_modes();

	test_dlist = true;
	pass = test_modes() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
