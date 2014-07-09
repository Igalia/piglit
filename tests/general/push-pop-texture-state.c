/**
 * Test pushing/popping of GL_TEXTURE_BIT state.
 *
 * Test case from fd.o bug #9833.
 * https://bugs.freedesktop.org/show_bug.cgi?id=9833
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);

	glPushAttrib(GL_TEXTURE_BIT);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glPopAttrib();
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
