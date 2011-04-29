/*
 * Copyright © 2010 Török Edwin
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

#include "piglit-util.h"

int piglit_width = 64;
int piglit_height = 64;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result piglit_display(void)
{
    GLint objID = glCreateProgram();
    /* check that it doesn't crash when linking empty shader */
    glLinkProgram(objID);
    glValidateProgram(objID);
    if (!piglit_link_check_status(objID))
	piglit_report_result(PIGLIT_FAIL);
    glUseProgram(objID);
    glUseProgram(0);
    glDeleteProgram(objID);
    piglit_report_result(PIGLIT_PASS);
    return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
    if (!GLEW_VERSION_2_0) {
	printf("Requires OpenGL 2.0\n");
	piglit_report_result(PIGLIT_SKIP);
    }
}
