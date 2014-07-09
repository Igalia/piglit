/*
 * Test case from fdo bug #23489
 * http://bugs.freedesktop.org/show_bug.cgi?id=23489
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	int i;

	for (i = 0; i < 100; i++) {
		glBegin(GL_LINES);
		glEnd();
	}
	glFlush();

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}
