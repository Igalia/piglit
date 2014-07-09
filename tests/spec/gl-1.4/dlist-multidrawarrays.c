/*
 * Copyright (C) 2014 VMware, Inc.
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
 * Test glMultiDrawArrays and similar functions in a display list.
 * glMultiDrawArrays is part of GL 1.4 and later.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 14;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const float verts[][2] = {
	{ -1.0f, -1.0f },
	{  1.0f, -1.0f },
	{  1.0f,  1.0f },
	{ -1.0f,  1.0f }
};


static const float white[3] = { 1.0f, 1.0f, 1.0f };
static const float black[3] = { 0.0f, 0.0f, 0.0f };


static bool
test_list(GLuint list, GLenum dlmode, const char *func)
{
	bool pass = true;
	const float *exp_color;

	assert(dlmode == GL_COMPILE || dlmode == GL_COMPILE_AND_EXECUTE);

	if (dlmode == GL_COMPILE_AND_EXECUTE) {
		/* the polygon should have been drawn during display
		 * list construction.
		 */
		exp_color = white;
	}
	else {
		/* the polygon should not have been drawn yet */
		exp_color = black;
	}
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     exp_color);
	if (!pass) {
		fprintf(stderr,
			"Compiling %s in display list failed for %s mode\n",
			func, piglit_get_gl_enum_name(dlmode));
		glDeleteLists(list, 1);
		return pass;
	}

	/* Now, call the list and make sure the polygon is rendered */
	glClear(GL_COLOR_BUFFER_BIT);
	glCallList(list);

	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, white);

	piglit_present_results();

	glDeleteLists(list, 1);

	if (!pass) {
		fprintf(stderr,
			"Calling %s in display list failed for %s mode\n",
			func, piglit_get_gl_enum_name(dlmode));
	}

	return pass;
}


static bool
test_MultiDrawArrays(GLenum dlmode)
{
	GLint first = 0, count = 4;
	GLuint list;

	glClear(GL_COLOR_BUFFER_BIT);

	list = glGenLists(1);
	glNewList(list, dlmode);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glMultiDrawArrays(GL_TRIANGLE_FAN, &first, &count, 1);
	glEndList();

	return test_list(list, dlmode, "glMultiDrawArrays");
}


static bool
test_MultiDrawElements(GLenum dlmode)
{
	const GLushort indices[] = { 3, 2, 1, 0 };
	const GLushort *multiIndices[] = { indices };
	GLint count = 4;
	GLuint list;

	glClear(GL_COLOR_BUFFER_BIT);

	list = glGenLists(1);
	glNewList(list, dlmode);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glMultiDrawElements(GL_TRIANGLE_FAN, &count, GL_UNSIGNED_SHORT,
			    (const GLvoid * const *) multiIndices, 1);
	glEndList();

	return test_list(list, dlmode, "glMultiDrawElements");
}


static bool
test_MultiModeDrawArraysIBM(GLenum dlmode)
{
	GLenum mode = GL_TRIANGLE_FAN;
	GLint first = 0, count = 4;
	GLuint list;

	glClear(GL_COLOR_BUFFER_BIT);

	list = glGenLists(1);
	glNewList(list, dlmode);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glMultiModeDrawArraysIBM(&mode, &first, &count, 1, sizeof(GLenum));
	glEndList();

	return test_list(list, dlmode, "glMultiModeDrawArraysIBM");
}


static bool
test_MultiModeDrawElementsIBM(GLenum dlmode)
{
	const GLushort indices[] = { 3, 2, 1, 0 };
	const GLushort *multiIndices[] = { indices };
	GLenum mode = GL_TRIANGLE_FAN;
	GLint count = 4;
	GLuint list;

	glClear(GL_COLOR_BUFFER_BIT);

	list = glGenLists(1);
	glNewList(list, dlmode);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glMultiModeDrawElementsIBM(&mode, &count, GL_UNSIGNED_SHORT,
				   (const GLvoid * const *) multiIndices,
				   1, sizeof(GLenum));
	glEndList();

	return test_list(list, dlmode, "glMultiModeDrawElementsIBM");
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_MultiDrawArrays(GL_COMPILE) && pass;
	pass = test_MultiDrawArrays(GL_COMPILE_AND_EXECUTE) && pass;
	pass = test_MultiDrawElements(GL_COMPILE) && pass;
	pass = test_MultiDrawElements(GL_COMPILE_AND_EXECUTE) && pass;
	if (piglit_is_extension_supported("GL_IBM_multimode_draw_arrays")) {
		pass = test_MultiModeDrawArraysIBM(GL_COMPILE) && pass;
		pass = test_MultiModeDrawArraysIBM(GL_COMPILE_AND_EXECUTE)
			&& pass;
		pass = test_MultiModeDrawElementsIBM(GL_COMPILE) && pass;
		pass = test_MultiModeDrawElementsIBM(GL_COMPILE_AND_EXECUTE)
			&& pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
