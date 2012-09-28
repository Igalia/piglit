/**
 * Test case from fd.o bug #9833.
 * https://bugs.freedesktop.org/show_bug.cgi?id=9833
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static int goterrors = 0;
	static int frame = 0;
	GLuint error;

	frame++;
	glClear(GL_COLOR_BUFFER_BIT);

	glPushAttrib(GL_TEXTURE_BIT);
	while ( (error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error 0x%0x occured after glPushAttrib!\n", error);
		goterrors++;
	}


	glPopAttrib();
	while ( (error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error 0x%0x occured after glPopAttrib!\n", error);
		goterrors++;
	}

	piglit_present_results();

	return goterrors ? PIGLIT_FAIL : PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
}
