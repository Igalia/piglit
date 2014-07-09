/*
 * Copyright © 2013 Intel Corporation
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

/* Test error behavior for GL_ARB_draw_indirect */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


static bool
check_binding_point(void)
{
	/* Check that the binding point exists, and the default
	 * binding must be zero
	 */

	GLint obj;
	glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &obj);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	if (obj != 0)
		return false;

	return true;
}


static bool
check_can_bind(void)
{
	/* Check that a buffer can be bound to the binding point.
	 * Does not *use* the buffer for anything.
	 */
	GLuint buf;
	GLint obj;
	glGenBuffers(1, &buf);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glBufferData(GL_DRAW_INDIRECT_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &obj);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	if (buf != obj)
		return false;

	return true;
}


static bool
check_draw_no_buffer_bound(void)
{
	/* In the core profile, an INVALID_OPERATION error is generated
	 * if zero is bound to DRAW_INDIRECT_BUFFER and DrawArraysIndirect
	 * or DrawElementsIndirect is called.
	 */

	/* Bind a buffer of indices; ensure we're hitting the correct
	 * error path with DrawElementsIndirect.
	 */
	GLuint ib;
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

	/* no indirect buffer */
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	glDrawArraysIndirect(GL_TRIANGLES, (GLvoid const *)0);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (GLvoid const *)0);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
check_draw_beyond_end(void)
{
	/* An INVALID_OPERATION error is generated if the commands source
	 * data beyond the end of the buffer object ..
	 */

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 5 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

	/* command is 4 * sizeof(GLuint); would read one GLuint beyond the end of the BO. */
	glDrawArraysIndirect(GL_TRIANGLES, (GLvoid const *)(2 * sizeof(GLuint)));

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	/* DrawElementsIndirect requires index buffer; bind the indirect buffer there too
	 * since it's handy; just to make sure we hit the right case. */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);

	/* command is 5 * sizeof(GLuint); would read one GLuint beyond the end. */
	glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (GLvoid const *)(1 * sizeof(GLuint)));

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
check_draw_misaligned(void)
{
	/* An INVALID_OPERATION error is generated
	 * .. or if <indirect> is not word aligned.
	 */

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

	glDrawArraysIndirect(GL_TRIANGLES, (GLvoid const *)1);	/* misaligned */
	
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
check_draw_elements_no_indices(void)
{
	/* If no element array buffer is bound, an INVALID_OPERATION
	 * error is generated.
	 */

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 5 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

	/* unbind indices */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (GLvoid const *)0);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
report(bool result, char const *name)
{
	piglit_report_subtest_result(result ? PIGLIT_PASS : PIGLIT_FAIL, "%s", name);
	return result;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint vao;
	piglit_require_extension("GL_ARB_draw_indirect");

	/* VAO is required since we're in core profile.
	 * Most of the subtests don't care about it.
	 */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	pass = report(check_binding_point(), "binding-point") && pass;
	pass = report(check_can_bind(), "can-bind") && pass;
	pass = report(check_draw_no_buffer_bound(), "draw-no-buffer-bound") && pass;
	pass = report(check_draw_beyond_end(), "draw-beyond-end") && pass;
	pass = report(check_draw_misaligned(), "draw-misaligned") && pass;
	pass = report(check_draw_elements_no_indices(), "draw-elements-no-indices") && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
