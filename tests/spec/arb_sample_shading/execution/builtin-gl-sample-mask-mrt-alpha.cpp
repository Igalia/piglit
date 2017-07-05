/*
 * Copyright (c) 2015 Intel Corporation
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

/** \file builtin-gl-sample-mask-mrt-alpha.cpp
 *
 *  This test verifies that assigning gl_SampleMask[] from the
 *  fragment shader works as expected in cases where the
 *  implementation is required to supply an additional alpha component
 *  previously written to a different color attachment to render to a
 *  non-zero attachment of a multisample FBO (e.g. while using
 *  alpha-to-coverage).
 */

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

#define NUM_SAMPLES 4

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 21;
        config.supports_gl_core_version = 31;

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
        config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static Fbo
make_fbo()
{
        FboConfig config(NUM_SAMPLES, piglit_width, piglit_height);
        config.num_rb_attachments = 0;
        config.num_tex_attachments = 2;
        config.tex_attachment[0] = GL_COLOR_ATTACHMENT0;
        config.tex_attachment[1] = GL_COLOR_ATTACHMENT1;

        Fbo fbo;
        fbo.setup(config);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                piglit_report_result(PIGLIT_FAIL);

        return fbo;
}

/**
 * Render to both texture attachments of the multisample fbo enabling
 * alpha-to-coverage to make the implementation pass the additional
 * alpha component from the first attachment when rendering into the
 * second.  The resulting sample mask will still be 5 as specified in
 * the fragment shader because an alpha value of 1.0 maps to the
 * coverage mask ~0.
 */
static bool
run_test(Fbo &fbo)
{
        const GLuint prog = piglit_build_simple_program(
                "#version 130\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   gl_Position = piglit_vertex;\n"
                "}\n",
                "#version 130\n"
                "#extension GL_ARB_sample_shading : enable\n"
                "\n"
                "out vec4 out_color[2];"
                "\n"
                "void main()\n"
                "{\n"
                "   gl_SampleMask[0] = 5;\n"
                "   out_color[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "   out_color[1] = vec4(1.0, 0.0, 0.0, 0.0);\n"
                "}\n");

        glUseProgram(prog);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.handle);
        glDrawBuffers(fbo.config.num_tex_attachments,
                      fbo.config.tex_attachment);
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        fbo.set_viewport();

        glClear(GL_COLOR_BUFFER_BIT);
        piglit_draw_rect(-1, -1, 2, 2);

        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glDeleteProgram(prog);

        return piglit_check_gl_error(GL_NO_ERROR);
}

/**
 * Resolve the red component of each sample from the texture
 * previously bound to color attachment i as the RGBA components of
 * the actual framebuffer.  Return true if only the first and third
 * samples were written according to the coverage mask set by the
 * shader.
 */
static bool
check(const Fbo &fbo, unsigned i)
{
        const float expected[] = { 1, 0, 1, 0 };
        const GLuint prog = piglit_build_simple_program(
                "#version 130\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   gl_Position = piglit_vertex;\n"
                "}\n",
                "#version 130\n"
                "#extension GL_ARB_texture_multisample : require\n"
                "\n"
                "uniform sampler2DMS tex;\n"
                "out vec4 out_color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   vec4 v;\n"
                "\n"
                "   for (int i = 0; i < 4; i++)\n"
                "      v[i] = texelFetch(tex, ivec2(gl_FragCoord.x,\n"
                "                                   gl_FragCoord.y), i).x;\n"
                "\n"
                "   out_color = v;\n"
                "}\n");

        glUseProgram(prog);
        glUniform1i(glGetUniformLocation(prog, "tex"), 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo.color_tex[i]);
        glViewport(0, 0, piglit_width, piglit_height);

        glClear(GL_COLOR_BUFFER_BIT);
        piglit_draw_rect(-1, -1, 2, 2);

        glDeleteProgram(prog);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
                                    expected)) {
                printf("  Attachment: %d\n", i);
                return false;
        }

        return true;
}

void
piglit_init(int argc, char **argv)
{
        piglit_require_extension("GL_ARB_texture_multisample");
        piglit_require_extension("GL_ARB_sample_shading");
        piglit_require_GLSL_version(130);

        Fbo fbo = make_fbo();

        if (run_test(fbo) && check(fbo, 0) && check(fbo, 1))
                piglit_report_result(PIGLIT_PASS);
        else
                piglit_report_result(PIGLIT_FAIL);
}

piglit_result
piglit_display()
{
        return PIGLIT_FAIL;
}
