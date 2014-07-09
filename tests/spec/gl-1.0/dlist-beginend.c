/*
 * Copyright (C) 2013 VMware, Inc.
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
 * Test some tricky cases of display lists and glBegin/End.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
static const GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
static const GLfloat black[] = {0.0, 0.0, 0.0, 0.0};


static bool
test_call_list_inside_begin_end(void)
{
	GLuint list;
	bool pass;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
	glEndList();

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glCallList(list);
	glEnd();

	pass = piglit_check_gl_error(GL_NO_ERROR);

	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "glCallList inside glBegin-glEnd");

	return pass;
}


static bool
test_call_list_inside_nested_begin_end(void)
{
	GLuint inner, outer;
	bool pass;

	glClear(GL_COLOR_BUFFER_BIT);

	inner = glGenLists(1);
	glNewList(inner, GL_COMPILE);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
	glEndList();

	outer = glGenLists(1);
	glNewList(outer, GL_COMPILE_AND_EXECUTE);
	glBegin(GL_QUADS);
	glCallList(inner);
	glEnd();
	glEndList();

	pass = piglit_check_gl_error(GL_NO_ERROR);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	glClear(GL_COLOR_BUFFER_BIT);
	glCallList(outer);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	glDeleteLists(inner, 1);
	glDeleteLists(outer, 1);

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "nested glCallList inside glBegin-glEnd");

	return pass;
}


static bool
test_illegal_rect_list_inside_begin_end(void)
{
	GLuint list;
	bool pass;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
	glColor4fv(red);
	glRectf(-1, -1, 1, 1);  /* illegal when called below */
	glEndList();

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glCallList(list);
	glEnd();

	/* The glRect command inside the display list should generate
	 * an error (and not draw a red rect!)
	 */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "illegal glRect inside glBegin-glEnd");

	return pass;
}


static bool
test_illegal_drawarrays_list_inside_begin_end(void)
{
	GLuint list;
	bool pass;
	static const GLfloat verts[4][2] = {
		{-1, -1}, {1, -1}, {1, 1}, {-1, 1} };

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
	glColor4fv(red);
	glDrawArrays(GL_QUADS, 0, 4);  /* this is illegal */
	glEndList();

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glCallList(list);
	glEnd();

	/* The glDrawArrays command inside the display list should generate
	 * an error (and not draw a red rect!)
	 */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	glDeleteLists(list, 1);
	glDisableClientState(GL_VERTEX_ARRAY);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"illegal glDrawArrays inside glBegin-glEnd");

	return pass;
}


/**
 * As above, but don't actually enable the vertex arrays.
 * This catches another Mesa bug.
 */
static bool
test_illegal_drawarrays_list_inside_begin_end2(void)
{
	GLuint list;
	bool pass;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glDrawArrays(GL_QUADS, 0, 4);  /* this is illegal */
	glEndList();

	glClear(GL_COLOR_BUFFER_BIT);
	glColor4fv(red);
	glBegin(GL_QUADS);
	glCallList(list);
	glEnd();

	/* The glDrawArrays command inside the display list should generate
	 * an error (and not draw a red rect!)
	 */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, black)
		&& pass;

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"illegal glDrawArrays inside glBegin-glEnd (2)");

	return pass;
}


static bool
test_separate_begin_vertex_end_lists(void)
{
	GLuint begin, vertex, end;
	bool pass;

	begin = glGenLists(1);
	glNewList(begin, GL_COMPILE);
	glBegin(GL_QUADS);
	glEndList();

	vertex = glGenLists(1);
	glNewList(vertex, GL_COMPILE);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
	glEndList();

	end = glGenLists(1);
	glNewList(end, GL_COMPILE);
	glEnd();
	glEndList();

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glCallList(begin);   /* error generated here */
	glCallList(vertex);
	glCallList(end);

	/* the glCallList(begin) call should have generated an error... */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	/* ... but we should still have drawn a green rect */
	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, green)
		&& pass;

	glDeleteLists(begin, 1);
	glDeleteLists(vertex, 1);
	glDeleteLists(end, 1);

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "separate glBegin-glVertex-glEnd lists");

	return pass;
}


static bool
test_illegal_begin_mode(void)
{
	GLuint list;
	bool pass;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE_AND_EXECUTE);
        glBegin(10000);
	glColor4fv(green);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1, 1);
	glVertex2f(-1, 1);
        glEnd();
	glEndList();

	/* the glBegin() call should have generated an error... */
	pass = piglit_check_gl_error(GL_INVALID_ENUM);

	glClear(GL_COLOR_BUFFER_BIT);
	glCallList(list);

	/* the glBegin() call should have generated an error again... */
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, black)
           && pass;

	piglit_present_results();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "illegal glBegin mode in display list");

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_call_list_inside_begin_end() && pass;
	pass = test_call_list_inside_nested_begin_end() && pass;
	pass = test_illegal_rect_list_inside_begin_end() && pass;
	pass = test_illegal_drawarrays_list_inside_begin_end() && pass;
	pass = test_illegal_drawarrays_list_inside_begin_end2() && pass;
	pass = test_separate_begin_vertex_end_lists() && pass;
        pass = test_illegal_begin_mode() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
