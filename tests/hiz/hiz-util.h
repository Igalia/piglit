/*
 * Copyright © 2011 Intel Corporation
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
 *
 * Authors:
 *     Chad Versace <chad.versace@intel.com>
 */

/**
 * \file hiz-util.h
 * \brief Utilities for HiZ tests
 * \author Chad Versace <chad.versace@intel.com>
 */

#pragma once

#include "piglit-util.h"

struct hiz_fbo_options {
	GLenum color_format;
	GLenum depth_format;
	GLenum stencil_format;
	GLenum depth_stencil_format;
};

static const GLfloat hiz_green[4]  = { 0.0, 1.0, 0.0, 1.0 };
static const GLfloat hiz_blue[4]   = { 0.0, 0.0, 1.0, 1.0 };
static const GLfloat hiz_grey[4]   = { 0.5, 0.5, 0.5, 1.0 };

/* These must be defines so that they can be used in constant initializers. */
#define hiz_green_z 0.25
#define hiz_blue_z  0.50
#define hiz_clear_z 0.875

/*
 * \brief Probe the color buffer.
 * \param expected_colors An array of 9 pointers, each to a float[4].
 * \return True if all probes pass.
 *
 * The color buffer is probed as follows.  Let the read buffer's dimension be
 * (w, h) and choose a tuple (i, j) where i and j are in {0, 1, 2}. Then the
 * expected color in the subrectangle
 *     {(x, y) | x in w / 3 * [i, i + 1] and y in h / 3 * [j, j + 1]}
 * is expected_colors[3 * j + i].
 */
bool hiz_probe_color_buffer(const float *expected_colors[]);

/*
 * \brief Probe the depth buffer.
 * \param expected_depths Array of 9 floats.
 * \return True if all probes pass.
 * \see hiz_probe_color_buffer()
 */
bool hiz_probe_depth_buffer(const float expected_depths[]);

GLuint hiz_make_fbo(const struct hiz_fbo_options *options);

/**
 * \brief For Valgrind's sake, delete the FBO and all attached renderbuffers.
 */
void hiz_delete_fbo(GLuint fbo);

/**
 * \brief Check that depth tests work correctly when rendering to an FBO.
 * \param options Perform the test with an FBO with the given formats.
 * \return True if test passed.
 */
bool hiz_run_test_depth_test_fbo(const struct hiz_fbo_options *options);

/**
 * Check that depth tests work correctly when rendering to the window
 * framebuffer.
 *
 * \param options Perform the test with an FBO with the given formats.
 * \return True if test passed.
 */
bool hiz_run_test_depth_test_window();
