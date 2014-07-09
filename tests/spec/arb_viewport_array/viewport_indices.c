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
 * Tests GL_ARB_viewport_array regarding the validity for the indices.
 * Use both valid and invalid parameters (index, first, count)
 * for these new API entry points:
 * glViewportArrayv, glViewportIndexedf, glViewportIndexedfv, glGetFloati_v.
 *
 * Also test that writing to an invalid viewport index for Viewport, DepthRange,
 * Scissor Box, Scissor Test does not modify any of the state for the valid
 * range of indices.
 *
 */

#include "piglit-util-gl.h"
#include <stdarg.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Test that ViewportArrayv, ViewportIndexedf(v), GetFloati_v give the
 * "expected_error" gl error.  Given the values for "first" and "count"
 * or "index" in range [first, first+count).
 */
static bool
check_vp_index(GLuint first, GLsizei count, GLenum expected_error)
{
	static const GLfloat v[] = {0.2, -2.3, 50.0, 1000.3};
	GLfloat *mv, vGet[4];
	unsigned int i;
	bool pass = true;
	const unsigned numIterate = (expected_error == GL_NO_ERROR)
		? count : 1;
	/* only iterate multiple indices for no error case */
	for (i = count; i > count - numIterate; i--) {
		glViewportIndexedf(first + i - 1, v[0], v[1], v[2], v[3]);
		pass = piglit_check_gl_error(expected_error) && pass;

		glViewportIndexedfv(first + i - 1, v);
		pass = piglit_check_gl_error(expected_error) && pass;

		glGetFloati_v(GL_VIEWPORT,first + i - 1, vGet);
		pass = piglit_check_gl_error(expected_error) && pass;
	}

	mv = malloc(sizeof(GLfloat) * 4 * count);
	if (mv == NULL)
		return false;
	for (i =0; i < count; i++) {
		mv[i*4] = v[0];
		mv[i*4 + 1] = v[1];
		mv[i*4 + 2] = v[2];
		mv[i*4 + 3] = v[3];
	}
	glViewportArrayv(first, count, mv);
	free(mv);
	pass = piglit_check_gl_error(expected_error) && pass;

	return pass;
}

/**
 * Test first + count or index valid invalid values.
 * Valid range is 0 thru (MAX_VIEWPORTS-1).
 * Also test the Enable, Disable, IsEnabled  with invalid index.
 */
static bool
test_vp_indices(GLint maxVP)
{
	bool pass = true;

	/**
	 * valid largest range viewport index
	 * OpenGL Core 4.3 Spec, section 13.6.1 ref:
	 *    "Multiple viewports are available and are numbered zero
	 *    through the value of MAX_VIEWPORTS minus one."
	 */
	if (!check_vp_index(0, maxVP, GL_NO_ERROR)) {
		printf("Got error for valid viewport range, max range=%u\n",
		       maxVP);
		pass = false;
	}
	/**
	 *  invalid count + first index for viewport
	 * OpenGL Spec Core 4.3 Spec, section 13.6.1 ref:
	 *     "An INVALID_VALUE error is generated if first + count
	 *     is greater than the value of MAX_VIEWPORTS."
	 */
	if (!check_vp_index(maxVP - 1, 2, GL_INVALID_VALUE)) {
		printf("Wrong error for invalid viewport range\n");
		pass = false;
	}
	/**
	 * invalid count for viewport
	 * OpenGL Spec Core 4.3 Spec, section 13.6.1 ref:
	 *    "An INVALID_VALUE error is generated if count is negative."
	 */
	glViewportArrayv(0, -1, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	return pass;
}

/**
 * Test values for viewports, depth_range and scissor boxes/test are preserved
 * with invalid indices.
 * OpenGL Core 4.3 Spec, section 13.6.1 ref:
 *    "Viewports whose indices lie outside the range [first, first + count)
 *    are not modified."
 */
static bool
test_preserve_invalid_index(GLint maxVP)
{
	bool pass = true;
	static const GLfloat vp[4] = {1.5555, 2.433, 3.777, 4.888};
	GLfloat vpGet[4];
	static const GLint sc[4] = {3, 9, 17, 23};
	GLint scGet[4];
	static const GLdouble dr[2] = {0.3333, 0.66666};
	GLdouble drGet[2];
	GLboolean scEnabled;
	int i;

	/* intialize all indices to know values */
	for (i = 0; i < maxVP; i++) {
		glViewportIndexedfv(i, vp);
		glDepthRangeIndexed(i, dr[0], dr[1]);
		glScissorIndexedv(i, sc);
		glEnablei(GL_SCISSOR_TEST, i);
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* set an illegal index and then test that no indices changed*/
	glViewportIndexedf(maxVP, 0.0, 0.0, 1.0, 1.0);
	glScissorIndexed(maxVP, 0, 0, 1, 1);
	glDepthRangeIndexed(maxVP, 0.0, 0.0);
	glDisablei(GL_SCISSOR_TEST, maxVP);

	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	for (i = 0; i < maxVP; i++) {
		glGetFloati_v(GL_VIEWPORT, i, vpGet);
		if (vpGet[0] != vp[0] || vpGet[1] != vp[1] || vpGet[2] != vp[2]
		    || vpGet[3] != vp[3]) {
			printf("Viewport index %d got erroneously changed\n",
			       i);
			pass = false;
		}
		glGetDoublei_v(GL_DEPTH_RANGE, i, drGet);
		if (drGet[0] != dr[0] || drGet[1] != dr[1]) {
			printf("DepthRange index %d got erroneously changed\n",
			       i);
			pass = false;
		}
		glGetIntegeri_v(GL_SCISSOR_BOX, i, scGet);
		if (scGet[0] != sc[0] || scGet[1] != sc[1] || scGet[2] != sc[2]
		    || scGet[3] != sc[3]) {
			printf("Scissor Box for index %d got erroneously changed\n",
			       i);
			pass = false;
		}
		scEnabled = glIsEnabledi(GL_SCISSOR_TEST, i);
		if (scEnabled == GL_FALSE) {
			printf("Scissor Test for index %d got erroneously changed\n",
			       i);
			pass = false;
		}
	}
	return pass;
}

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

	piglit_require_extension("GL_ARB_viewport_array");

	glGetIntegerv(GL_MAX_VIEWPORTS, &maxVP);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = test_preserve_invalid_index(maxVP) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = test_vp_indices(maxVP) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
