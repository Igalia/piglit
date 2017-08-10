/*
 * BEGIN_COPYRIGHT -*- glean -*-
 *
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 * Copyright (C) 2015  Intel Corporation  All Rights Reserved.
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
 *
 * END_COPYRIGHT
 */

/** @file two-sided-stencil.c
 *
 *  Test two-sided stencil extensions
 *
 *  	This test could be better:
 * 	1. Generate random state vectors, render and compare to expected values
 * 	2. Exercise separate front/back reference values and masks for the
 *    	EXT and GL2 variations.
 *
 * 	Note: Must check writeMask of set_stencil_state to make sure it's correct
 *
 *   	Authors:
 *     	Brian Paul <brianp@valinux.com>
 *     	Adapted to Piglit by Juliet Fru <julietfru@gmail.com>, September 2015.
 */

#include "piglit-util-gl.h"

#include <assert.h>
#include <string.h>


PIGLIT_GL_TEST_CONFIG_BEGIN config.supports_gl_compat_version = 20;

config.window_visual =
	PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH |
	PIGLIT_GL_VISUAL_STENCIL;
config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/*  two-sided methods. */
#define ATI 1
#define EXT 2
#define GL2 3

GLint stencil_bits, stencil_max;


static bool
have_stencil_wrap(void)
{
	if (piglit_get_gl_version() >= 2.0) {
		return true;
	} else if (piglit_is_extension_supported("GL_EXT_stencil_wrap")) {
		return true;
	}
	return false;
}


/* Draw four quads:
 * Bottom row uses GL_CCW
 * Top row uses GL_CW
 * Left column is front-facing
 * Right column is back-facing
 * Check the values in the stencil buffer to see if they match
 * the expected values.
 */
static bool
render_test(GLuint expectedFront, GLuint expectedBack)
{
	GLint x0 = 0;
	GLint x1 = piglit_width / 2;
	GLint x2 = piglit_width;
	GLint y0 = 0;
	GLint y1 = piglit_width / 2;
	GLint y2 = piglit_width;

	glFrontFace(GL_CCW);	/* this the GL default */

	/* lower left quad = front-facing */
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x0, y0);
	glVertex2f(x1, y0);
	glVertex2f(x1, y1);
	glVertex2f(x0, y1);
	glEnd();

	/* lower right quad = back-facing */
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y0);
	glEnd();

	glFrontFace(GL_CW);

	/* upper left quad = front-facing */
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x0, y1);
	glVertex2f(x0, y2);
	glVertex2f(x1, y2);
	glVertex2f(x1, y1);
	glEnd();

	/* upper right quad = back-facing */
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();

	GLint midXleft = (x0 + x1) / 2;
	GLint midXright = (x1 + x2) / 2;
	GLint midYlower = (y0 + y1) / 2;
	GLint midYupper = (y1 + y2) / 2;
	GLuint lowerLeftVal, lowerRightVal;
	GLuint upperLeftVal, upperRightVal;

	glReadPixels(midXleft, midYlower, 1, 1,
		     GL_STENCIL_INDEX, GL_UNSIGNED_INT, &lowerLeftVal);
	glReadPixels(midXright, midYlower, 1, 1,
		     GL_STENCIL_INDEX, GL_UNSIGNED_INT, &lowerRightVal);

	glReadPixels(midXleft, midYupper, 1, 1,
		     GL_STENCIL_INDEX, GL_UNSIGNED_INT, &upperLeftVal);
	glReadPixels(midXright, midYupper, 1, 1,
		     GL_STENCIL_INDEX, GL_UNSIGNED_INT, &upperRightVal);

	if (lowerLeftVal != upperLeftVal) {
		printf(" FAIL:\n"
		       "\tLower-left value (%d) doesn't match upper-left "
		       "value (%d).\n \t Looks like a front/back "
		       "orientation bug.\n", lowerLeftVal, upperLeftVal);
		return false;
	}

	if (lowerRightVal != upperRightVal) {
		printf(" FAIL:\n\tLower-right value (%d) doesn't match"
		       " upper-right value (%d).\n\tLooks like "
		       "a front/back-face orientation bug.\n", lowerRightVal,
		       upperRightVal);
		return false;
	}


	if (lowerLeftVal != expectedFront) {
		printf("FAIL:\n\tExpected front-face stencil value is "
		       "%d but found %d \n", expectedFront, lowerLeftVal);
		return false;
	} else if (lowerRightVal != expectedBack) {
		printf("FAIL:\n\tExpected back-face stencil value is "
		       "%d but found %d \n", expectedBack, lowerRightVal);
		return false;
	} else {
		return true;
	}
}


static bool
compare_state(int method, GLenum found, GLenum expected, const char *msg)
{
	if (found != expected) {
		printf(" FAIL:\n\tQuery of %s state failed for ", msg);
		switch (method) {
		case ATI:
			printf("GL_ATI_separate_stencil\n");
			break;
		case EXT:
			printf("GL_EXT_stencil_two_side\n");
			break;
		case GL2:
			printf("GL 2.x two-sided stencil\n");
			break;
		default:
			printf("\n");
			assert(0);
		}

		printf("\tFound 0x%x, expected 0x%x\n", found, expected);
		return false;
	}
	return true;
}


/* Set stencil state, plus read it back and check that it's correct.
 * Note: we only test with one reference value and one mask value
 * even though EXT and GL2 support separate front/back refs/masks
 */
static bool
set_stencil_state(int method,
		  GLenum frontStencilFail,
		  GLenum backStencilFail,
		  GLenum frontZFail,
		  GLenum backZFail,
		  GLenum frontZPass,
		  GLenum backZPass,
		  GLenum frontFunc,
		  GLenum backFunc,
		  GLint frontRef,
		  GLint backRef,
		  GLuint frontMask,
		  GLuint backMask,
		  GLuint frontWriteMask, GLuint backWriteMask)
{
	GLint get_frontStencilFail;
	GLint get_backStencilFail;
	GLint get_frontZFail;
	GLint get_backZFail;
	GLint get_frontZPass;
	GLint get_backZPass;
	GLint get_frontFunc;
	GLint get_backFunc;
	GLint get_frontRef;
	GLint get_backRef;
	GLint get_frontMask;
	GLint get_backMask;
	GLint get_frontWriteMask;
	GLint get_backWriteMask;
	GLint twoEnabled;

	switch (method) {
	case ATI:
		assert(frontRef == backRef);
		assert(frontMask == backMask);
		assert(frontWriteMask == backWriteMask);

		/* set state */
		glStencilOpSeparateATI(GL_FRONT,
				       frontStencilFail,
				       frontZFail, frontZPass);

		glStencilOpSeparateATI(GL_BACK,
				       backStencilFail, backZFail, backZPass);

		glStencilFuncSeparateATI(frontFunc, backFunc, frontRef,
					 frontMask);

		glStencilMask(frontWriteMask);

		/* get state */
		glGetIntegerv(GL_STENCIL_FAIL, &get_frontStencilFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &get_frontZFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &get_frontZPass);
		glGetIntegerv(GL_STENCIL_FUNC, &get_frontFunc);
		glGetIntegerv(GL_STENCIL_REF, &get_frontRef);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &get_frontMask);
		glGetIntegerv(GL_STENCIL_WRITEMASK, &get_frontWriteMask);

		glGetIntegerv(GL_STENCIL_BACK_FUNC_ATI, &get_backFunc);
		glGetIntegerv(GL_STENCIL_BACK_FAIL_ATI, &get_backStencilFail);
		glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI,
			      &get_backZFail);
		glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI,
			      &get_backZPass);
		get_backRef = get_frontRef;
		get_backMask = get_frontMask;
		get_backWriteMask = get_frontWriteMask;
		twoEnabled = GL_TRUE;
		break;

	case EXT:
		/* set state */
		glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

		glActiveStencilFaceEXT(GL_FRONT);
		glStencilOp(frontStencilFail, frontZFail, frontZPass);
		glStencilFunc(frontFunc, frontRef, frontMask);
		glStencilMask(frontWriteMask);

		glActiveStencilFaceEXT(GL_BACK);
		glStencilOp(backStencilFail, backZFail, backZPass);
		glStencilFunc(backFunc, backRef, backMask);
		glStencilMask(backWriteMask);

		/* get state */
		glActiveStencilFaceEXT(GL_FRONT);
		glGetIntegerv(GL_STENCIL_FAIL, &get_frontStencilFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &get_frontZFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &get_frontZPass);
		glGetIntegerv(GL_STENCIL_FUNC, &get_frontFunc);
		glGetIntegerv(GL_STENCIL_REF, &get_frontRef);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &get_frontMask);
		glGetIntegerv(GL_STENCIL_WRITEMASK, &get_frontWriteMask);
		glActiveStencilFaceEXT(GL_BACK);
		glGetIntegerv(GL_STENCIL_FAIL, &get_backStencilFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &get_backZFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &get_backZPass);
		glGetIntegerv(GL_STENCIL_FUNC, &get_backFunc);
		glGetIntegerv(GL_STENCIL_REF, &get_backRef);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &get_backMask);
		glGetIntegerv(GL_STENCIL_WRITEMASK, &get_backWriteMask);
		glGetIntegerv(GL_STENCIL_TEST_TWO_SIDE_EXT, &twoEnabled);
		break;

	case GL2:
		/* set state */
		glStencilOpSeparate(GL_FRONT,
				    frontStencilFail, frontZFail, frontZPass);
		glStencilOpSeparate(GL_BACK,
				    backStencilFail, backZFail, backZPass);
		glStencilFuncSeparate(GL_FRONT, frontFunc, frontRef,
				      frontMask);
		glStencilFuncSeparate(GL_BACK, backFunc, backRef, backMask);
		glStencilMaskSeparate(GL_FRONT, frontWriteMask);
		glStencilMaskSeparate(GL_BACK, backWriteMask);

		/* get state */
		glGetIntegerv(GL_STENCIL_FAIL, &get_frontStencilFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &get_frontZFail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &get_frontZPass);
		glGetIntegerv(GL_STENCIL_FUNC, &get_frontFunc);
		glGetIntegerv(GL_STENCIL_REF, &get_frontRef);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &get_frontMask);
		glGetIntegerv(GL_STENCIL_WRITEMASK, &get_frontWriteMask);

		glGetIntegerv(GL_STENCIL_BACK_FUNC, &get_backFunc);
		glGetIntegerv(GL_STENCIL_BACK_FAIL, &get_backStencilFail);
		glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL,
			      &get_backZFail);
		glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS,
			      &get_backZPass);
		glGetIntegerv(GL_STENCIL_BACK_REF, &get_backRef);
		glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &get_backMask);
		glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &get_backWriteMask);
		twoEnabled = GL_TRUE;
		break;

	default:
		assert(0);
	}

	/* mask off bits we don't care about */
	get_frontMask &= stencil_max;
	frontMask &= stencil_max;
	get_backMask &= stencil_max;
	backMask &= stencil_max;
	get_frontWriteMask &= stencil_max;
	frontWriteMask &= stencil_max;
	get_backWriteMask &= stencil_max;
	backWriteMask &= stencil_max;


	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		return false;
	}

	/* see if state-get matches state-set */

	if (!compare_state(method, get_frontStencilFail, frontStencilFail,
			   "front stencil fail"))
		return false;

	if (!compare_state(method, get_backStencilFail, backStencilFail,
			   "back stencil fail"))
		return false;

	if (!compare_state(method, get_frontZFail, frontZFail,
			   "front Z fail"))
		return false;

	if (!compare_state(method, get_backZFail, backZFail, "back Z fail"))
		return false;

	if (!compare_state(method, get_frontZPass, frontZPass,
			   "front Z pass"))
		return false;

	if (!compare_state(method, get_backZPass, backZPass, "back Z pass"))
		return false;

	if (!compare_state(method, get_frontFunc, frontFunc,
			   "front stencil func"))
		return false;

	if (!compare_state(method, get_backFunc, backFunc,
			   "back stencil func"))
		return false;

	if (!compare_state(method, get_frontRef, frontRef,
			   "front stencil ref"))
		return false;

	if (!compare_state(method, get_backRef, backRef, "back stencil ref"))
		return false;

	if (!compare_state(method, get_frontMask, frontMask,
			   "front stencil mask"))
		return false;

	if (!compare_state(method, get_backMask, backMask,
			   "back stencil mask"))
		return false;

	if (!compare_state(method, get_frontWriteMask, frontWriteMask,
			   "front stencil writemask"))
		return false;

	if (!compare_state(method, get_backWriteMask, backWriteMask,
			   "back stencil writemask"))
		return false;

	if (!compare_state(method, twoEnabled, GL_TRUE, "two-side enable"))
		return false;

	return true;
}


static bool
set_stencil_state2(int method,
		   GLenum frontStencilFail,
		   GLenum backStencilFail,
		   GLenum frontZFail,
		   GLenum backZFail,
		   GLenum frontZPass,
		   GLenum backZPass,
		   GLenum frontFunc,
		   GLenum backFunc,
		   GLint ref,
		   GLuint mask,
		   GLuint writeMask)
{
	return set_stencil_state(method, frontStencilFail,
				 backStencilFail, frontZFail, backZFail,
				 frontZPass, backZPass, frontFunc, backFunc,
				 ref,	/* frontRef */
				 ref,	/* backRef */
				 mask,	/* frontMask */
				 mask,	/* backMask */
				 writeMask,	/* frontWriteMask */
				 writeMask);	/* backWriteMask */
}


void
reset_stencil_state(int method)
{
	switch (method) {
	case ATI:
		break;
	case EXT:
		glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
		glActiveStencilFaceEXT(GL_FRONT);
		break;
	case GL2:
		break;
	default:
		assert(0);
	}
}


static bool
test_stencil(int method)
{
	bool pass;

	glEnable(GL_STENCIL_TEST);

	/**
	 * No depth testing
	 */
	glDisable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/* set stencil buffer vals to 5 */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_REPLACE, GL_REPLACE,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  5, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(5, 5);
	reset_stencil_state(method);
	if (!pass)
		return false;

	/* incr front val to 6, decr back val to 4 */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_INCR, GL_DECR,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  5, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(6, 4);
	reset_stencil_state(method);
	if (!pass)
		return false;

	/* if front==6, keep
	 * if back<6, replace with zero
	 * final: front=6, back=0
	 */
	pass = set_stencil_state2(method, GL_KEEP, GL_ZERO,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_KEEP, GL_KEEP,	/* z pass */
				  GL_EQUAL, GL_LESS,	/* stencil func */
				  6, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(6, 0);
	reset_stencil_state(method);
	if (!pass)
		return false;

	/* if front!=10, keep, else decr
	 * if back<10, keep, else incr
	 * final: front=6, back=1
	 */
	pass = set_stencil_state2(method, GL_DECR, GL_INCR,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_KEEP, GL_KEEP,	/* z pass */
				  GL_NOTEQUAL, GL_LESS,	/* stencil func */
				  10, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(6, 1);
	reset_stencil_state(method);
	if (!pass)
		return false;

	if (method != ATI) {
		/* if front!=10, keep, else decr
		 * if back<10, keep, else incr
		 * final: front=6, back=1
		 */
		pass = set_stencil_state(method, GL_DECR, GL_INCR,	/* stencil fail */
					 GL_KEEP, GL_KEEP,	/* z fail */
					 GL_REPLACE, GL_REPLACE,	/* z pass */
					 GL_ALWAYS, GL_ALWAYS,	/* stencil func */
					 0xf6, 0xf1,	/* ref */
					 0xff, 0xff,	/* mask */
					 0x60, 0x10);	/* writeMask */
		if (pass)
			pass = render_test(0x66, 0x11);
		reset_stencil_state(method);
		if (!pass)
			return false;
	}

	/* reset write mask for clear */
	set_stencil_state(method, GL_KEEP, GL_KEEP,	/* stencil fail */
			  GL_KEEP, GL_KEEP,	/* z fail */
			  GL_REPLACE, GL_REPLACE,	/* z pass */
			  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
			  0, 0, ~0, ~0, ~0, ~0);

	/* ============================================================
	 * Now begin tests with depth test
	 */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClear(GL_COLOR_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* set stencil buffer vals to 7, set Z values */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_REPLACE, GL_REPLACE,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  7, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(7, 7);
	reset_stencil_state(method);
	if (!pass)
		return false;


	/* GL_LESS test should fail everywhere
	 * decr front to 5, incr back to 2
	 */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_DECR, GL_INCR,	/* z fail */
				  GL_KEEP, GL_KEEP,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  99, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(6, 8);
	reset_stencil_state(method);
	if (!pass)
		return false;


	/* set depth test = GL_EQUAL
	 * Z test should pass everywhere
	 * set front to 3
	 * decr back to 7
	 */
	glDepthFunc(GL_EQUAL);
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_REPLACE, GL_DECR,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  3, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(3, 7);
	reset_stencil_state(method);
	if (!pass)
		return false;


	/* incr front to 4 (by z pass), decr back to 6 (by stencil fail) */
	pass = set_stencil_state2(method, GL_DECR, GL_DECR,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_INCR, GL_REPLACE,	/* z pass */
				  GL_EQUAL, GL_EQUAL,	/* stencil func */
				  3, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(4, 6);
	reset_stencil_state(method);
	if (!pass)
		return false;


	/* ============================================================
	 * Disable depth test
	 */
	glDisable(GL_DEPTH_TEST);

	/* test stencil value mask
	 * only test bit 1 in stencil values
	 * if !(front&0x2 == 15&0x2), decr to 3 (should happen)
	 * if !(back&0x2 == 15&0x2), incr to 7 (should not happen)
	 */
	pass = set_stencil_state2(method, GL_DECR, GL_INCR,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_KEEP, GL_KEEP,	/* z pass */
				  GL_EQUAL, GL_EQUAL,	/* stencil func */
				  15, 0x2, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(3, 6);
	reset_stencil_state(method);
	if (!pass)
		return false;

	/* ============================================================
	 * Test common two-sided stencil modes for shadow volume rendering
	 * Requires stencil /- wrap feature.
	 */

	if (!have_stencil_wrap())
		return true;

	glClear(GL_COLOR_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* "traditional / Z-pass" method:
	 * front face: incr on zpass
	 * back face: decr on zpass
	 * both front and back Z-test should pass here
	 */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_KEEP, GL_KEEP,	/* z fail */
				  GL_INCR_WRAP_EXT, GL_DECR_WRAP_EXT,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  0, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(1, stencil_max);
	reset_stencil_state(method);
	if (!pass)
		return false;


	/* "Z-fail" method:
	 * front face: decr on zfail
	 * back face: incr on zfail
	 * both front and back Z-test should fail here
	 */
	pass = set_stencil_state2(method, GL_KEEP, GL_KEEP,	/* stencil fail */
				  GL_DECR_WRAP_EXT, GL_INCR_WRAP_EXT,	/* z fail */
				  GL_KEEP, GL_KEEP,	/* z pass */
				  GL_ALWAYS, GL_ALWAYS,	/* stencil func */
				  0, ~0, ~0);	/* ref, mask, set write_mask to ~0 */
	if (pass)
		pass = render_test(0, 0);
	reset_stencil_state(method);
	if (!pass)
		return false;


	return true;
}


void
piglit_init(int argc, char **argv)
{
	/* no initialization */
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* how many stencil bits (we assume at least 8 above) */
	glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
	stencil_max = (1 << stencil_bits) - 1;
	assert(stencil_bits >= 8);

	glViewport(0, 0, piglit_width, piglit_width);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_width, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (piglit_is_extension_supported("GL_ATI_separate_stencil")) {
		pass = test_stencil(ATI) && pass;
	}

	if (piglit_is_extension_supported("GL_EXT_stencil_two_side")) {
		pass = test_stencil(EXT) && pass;
	}

	if (piglit_get_gl_version() >= 2.0) {
		pass = test_stencil(GL2) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
