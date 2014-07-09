#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char *argv[])
{
    GLuint id;

    piglit_require_gl_version(15);

    piglit_require_extension("GL_ARB_vertex_buffer_object");

    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, 0, NULL, GL_STATIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glMapBuffer(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB); /* CRASH! */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
