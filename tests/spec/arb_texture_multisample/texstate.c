/*
 * Copyright © 2013 Chris Forbes
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
    return PIGLIT_FAIL;
}

static bool
check_texlevelparameter_int(GLuint target, GLuint level,
        char const *name, GLuint pname, GLint expected_value)
{
    GLint actual_value;
    glGetTexLevelParameteriv(target, level, pname, &actual_value);
    if (!piglit_check_gl_error(GL_NO_ERROR))
        return false;

    if (actual_value != expected_value) {
        printf("Expected %s value of %d but got %d\n",
                name, expected_value, actual_value);
        return false;
    }

    return true;
}

void
piglit_init(int argc, char **argv)
{
    GLuint tex2d;
    bool pass = true;
    piglit_require_extension("GL_ARB_texture_multisample");

    /* check that new image state required by ARB_texture_multisample
     * exists, and has correct defaults. Tests against a non-multisample
     * texture target, since this state exists on all images. */

    glGenTextures(1, &tex2d);
    glBindTexture(GL_TEXTURE_2D, tex2d);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    pass = check_texlevelparameter_int(GL_TEXTURE_2D, 0, "GL_TEXTURE_SAMPLES",
            GL_TEXTURE_SAMPLES, 0) && pass;
    pass = check_texlevelparameter_int(GL_TEXTURE_2D, 0, "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS",
            GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, GL_TRUE) && pass;

    piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
