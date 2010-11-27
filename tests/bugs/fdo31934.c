#include "piglit-util.h"

static int Automatic = 0;

int main (int argc, char *argv[])
{
    GLuint id;

    glutInit(&argc, argv);
    if (argc == 2 && !strcmp(argv[1], "-auto"))
        Automatic = 1;
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(512,512);
    glutCreateWindow("fdo31934");

    glewInit();

    if (!GLEW_VERSION_1_5) {
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

    if (Automatic)
        printf("PIGLIT: {'result': 'pass' }\n");

    return 0;
}
