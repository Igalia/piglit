/*
 * Copyright © 2019 Intel Corporation
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

/*
 * Drawing with dual source blending enabled while fragment shader
 * doesn't write into src1 is undefined but it should not hang GPU.
 * It hanged Intel gen8+ GPUs with depth test enabled.
 *
 * To detect a hang we clear the window with red, enable dual source blend,
 * draw with shader which doesn't write to src1 and discards every pixel
 * then clear the window with green and check if this clearing succeeded to
 * check that we didn't lost GPU after the draw.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=107088
 *
 * \author Danylo Piliaiev <danylo.piliaiev@gmail.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
    config.supports_gl_compat_version = 30;
#else // PIGLIT_USE_OPENGLES3
    config.supports_gl_es_version = 30;
#endif
    config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH;
    config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
static const char *vs_text =
    "#version 130\n"
    "in vec4 vertex;\n"
    "void main() { gl_Position = vertex; }\n"
    ;

static const char *fs_text =
    "#version 130\n"
    "void main() {\n"
    "    discard;\n"
    "}\n"
    ;
#else // PIGLIT_USE_OPENGLES3
static const char *vs_text =
    "#version 300 es\n"
    "in vec4 piglit_vertex;\n"
    "void main() { gl_Position = piglit_vertex; }\n"
    ;

static const char *fs_text =
    "#version 300 es\n"
    "void main() {\n"
    "    discard;\n"
    "}\n"
    ;
#endif

enum piglit_result
piglit_display(void)
{
    const float green[] = {0.0, 1.0, 0.0};

    glClearColor(1.0, 0.0, 0.0, 1.0);
#ifdef PIGLIT_USE_OPENGL
    glClearDepth(1.0);
#else
    glClearDepthf(1.0);
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    piglit_draw_rect(0, 0, 1, 1);

    bool pass = piglit_check_gl_error(GL_NO_ERROR);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
    pass = piglit_probe_pixel_rgb(1, 1, green) && pass;

    glFinish();

    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
#ifdef PIGLIT_USE_OPENGL
    piglit_require_extension("GL_ARB_blend_func_extended");
#else // PIGLIT_USE_OPENGLES3
    piglit_require_extension("GL_EXT_blend_func_extended");
#endif

    GLint max_dual_source;
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_dual_source);

    if (max_dual_source < 1) {
        fprintf(stderr,
            "ARB_blend_func_extended requires "
            "GL_MAX_DUAL_SOURCE_DRAW_BUFFERS >= 1. "
            "Only got %d!\n",
            max_dual_source);
        piglit_report_result(PIGLIT_FAIL);
    }

    GLuint prog = piglit_build_simple_program(vs_text, fs_text);
    glUseProgram(prog);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_SRC1_COLOR, GL_ONE, GL_ZERO);

    glEnable(GL_DEPTH_TEST);
}
