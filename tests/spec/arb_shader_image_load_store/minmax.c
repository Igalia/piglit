/*
 * Copyright (C) 2014 Intel Corporation
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

/** @file minmax.c
 *
 * Test the minimum values for the implementation limits specified by
 * the ARB_shader_image_load_store extension.
 */

#include "common.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;
        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
        int ver = piglit_get_gl_version();

        piglit_require_extension("GL_ARB_shader_image_load_store");

        piglit_print_minmax_header();

        piglit_test_min_int(GL_MAX_IMAGE_UNITS, 8);
        piglit_test_min_int(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS, 8);
        piglit_test_min_int(GL_MAX_IMAGE_SAMPLES, 0);
        piglit_test_min_int(GL_MAX_VERTEX_IMAGE_UNIFORMS, 0);
        piglit_test_min_int(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, 8);
        if (ver >= 32)
                piglit_test_min_int(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, 0);
        if (ver >= 40) {
                piglit_test_min_int(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, 0);
                piglit_test_min_int(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, 0);
        }
        if (ver >= 43)
                piglit_test_min_int(GL_MAX_COMPUTE_IMAGE_UNIFORMS, 8);
        piglit_test_min_int(GL_MAX_COMBINED_IMAGE_UNIFORMS, 8);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                piglit_report_result(PIGLIT_FAIL);

        piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
