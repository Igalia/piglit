/*
 * Copyright © 2013 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */
/**
 * Test valid  and invalid queries using glGetFloati_v and glGetDoublei_v.
 * NOTE: "index" parameter validity is tested in the depthrange-indices
 * and viewport-indices tests for the glGet*i_v(). The "bounds" test
 * does test some valid queries using glGet*i_v().
 * Also test GL_SCISSOR_TEST default value and settable value can be
 * correctly queried.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint maxVP;
	GLboolean scEnabled;
	GLdouble vald[4] = {0.0, 0.0, 0.0, 0.0};
	GLfloat valf[4] = {0.0, 0.0, 0.0, 0.0};
	GLint vali[4] = {0, 0, 0, 0};
	int i;
	const GLenum tokens[] = {GL_MAX_VIEWPORTS, GL_VIEWPORT_SUBPIXEL_BITS,
			   GL_VIEWPORT_BOUNDS_RANGE, GL_LAYER_PROVOKING_VERTEX,
			   GL_VIEWPORT_INDEX_PROVOKING_VERTEX};
	const GLenum indexedTokens[] = {GL_VIEWPORT, GL_DEPTH_RANGE,
					GL_SCISSOR_BOX};

	piglit_require_extension("GL_ARB_viewport_array");

	glGetIntegerv(GL_MAX_VIEWPORTS, &maxVP);
	/**
	 * Test for invalid (non-indexed "pname") parameters with GetFloati_v
	 * and GetDoublei_v
	 * NOTE: "index" parameter validity is tested in the depthrange-indices
	 * and viewport-indices tests.
	 * OpenGL 4.3 Core section 22.1 ref:
	 *     "An INVALID_ENUM error is generated if target is not indexed
	 *     state queriable with these commands."
	 */

	for (i = 0; i < ARRAY_SIZE(tokens); i++) {
		glGetFloati_v(tokens[i], 1, valf);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
		glGetDoublei_v(tokens[i], 1, vald);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	}

	/**
	 * Test default value for SCISSOR_TEST via query.
	 * OpenGL 4.3 Core section 13.6.1 ref:
	 *    "Initially, the scissor test is disabled for all viewports."
	 */
	for (i = 0; i < maxVP; i++) {
		scEnabled = glIsEnabledi(GL_SCISSOR_TEST, i);
		if (scEnabled == GL_TRUE) {
			printf("scissor test default value wrong for idx %d\n",
			       i);
			pass = false;
		}
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/**
	 * Test settable value for SCISSOR_TEST can be queried
	 */
	/* first setup initial values */
	for (i = 0; i < maxVP; i++) {
		scEnabled = i & 0x1;
		if (scEnabled)
			glEnablei(GL_SCISSOR_TEST, i);
		else
			glDisablei(GL_SCISSOR_TEST, i);
	}
	/* test can query these values */
	for (i = 0; i < maxVP; i++) {
		scEnabled = i & 0x1;
		if (scEnabled != glIsEnabledi(GL_SCISSOR_TEST, i)) {
			pass = false;
			printf("Wrong queried value for GL_SCISSOR_TEST, idx=%d\n",
			       i);
		}
	}

	/**
	 * Test for valid "pname" parameter used with various forms of glGet.
	 * return the same data.
	 */
	glViewport(1, 2, 30, 40);
	glDepthRange(0.25, 0.75);
	glScissor(3, 4, 50, 60);
	for (i =0; i < ARRAY_SIZE(indexedTokens); i++) {
		glGetFloati_v(indexedTokens[i], 1, valf);
		glGetDoublei_v(indexedTokens[i], 1, vald);
		glGetIntegeri_v(indexedTokens[i], 1, vali);
		if (valf[0] != vald[0] || valf[1] != vald[1] ||
		    valf[2] != vald[2] || valf[3] != vald[3]) {
			pass = false;
			printf("mismatched valf and vald for %s\n",
			       piglit_get_gl_enum_name(indexedTokens[i]));
			printf("valf[0-3]= %f %f %f %f\n", valf[0], valf[1],
			       valf[2], valf[3]);
			printf("vald[0-3] = %f %f %f %f\n", vald[0], vald[1],
			       vald[2], vald[3]);
		}
		if ((int) (valf[0] + 0.5) != vali[0] ||
		    (int) (valf[1] + 0.5) != vali[1] ||
		    (int) (valf[2] + 0.5) != vali[2] ||
		    (int) (valf[3] + 0.5) != vali[3]) {
			pass = false;
			printf("mismatched valf and vali for %s\n",
			       piglit_get_gl_enum_name(indexedTokens[i]));
			printf("valf[0-3]= %f %f %f %f\n", valf[0], valf[1],
			       valf[2], valf[3]);
			printf("vali[0-3] = %d %d %d %d\n", vali[0], vali[1],
			       vali[2], vali[3]);
		}
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
