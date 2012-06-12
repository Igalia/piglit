#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    512 /*window_width*/,
    512 /*window_height*/,
    GLUT_RGBA)

void
piglit_init(int argc, char *argv[])
{
    GLuint id;

    if (piglit_get_gl_version() < 15) {
        printf("Requires OpenGL 1.5\n");
        piglit_report_result(PIGLIT_SKIP);
    }

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
