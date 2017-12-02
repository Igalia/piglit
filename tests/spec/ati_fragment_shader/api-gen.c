/*
 * Copyright © 2017 Miklós Máté
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

/**
 * Tests basic API functionality for GL_ATI_fragment_shader:
 * - generating names
 * - deleting named shaders
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	unsigned id;
	bool pass = true;

	piglit_require_extension("GL_ATI_fragment_shader");

	/* gen some shaders */
	id = glGenFragmentShadersATI(3);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (id == 0)
		pass = false;

	/* delete them */
	glDeleteFragmentShaderATI(id);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glDeleteFragmentShaderATI(id+1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glDeleteFragmentShaderATI(id+2);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* gen a few more, leave them hanging */
	id = glGenFragmentShadersATI(3);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* bind allocates the name, no need for gen */
	glBindFragmentShaderATI(42);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glDeleteFragmentShaderATI(42);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* bind an other one, leave it hanging */
	glBindFragmentShaderATI(43);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* try to delete a non-existent one */
	glDeleteFragmentShaderATI(628);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
