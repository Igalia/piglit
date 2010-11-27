#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glut.h>

int main (int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(512,512);
    glutCreateWindow("");

    GLuint id;
    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, 0, NULL, GL_STATIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glMapBuffer(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB); /* CRASH! */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    return 0;
}
