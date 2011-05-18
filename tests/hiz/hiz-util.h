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

static const GLfloat hiz_green_z  = 0.25;
static const GLfloat hiz_blue_z   = 0.50;
static const GLfloat hiz_clear_z  = 0.875;

/**
 * \brief Probe the scene drawn by hiz_draw_rects().
 * \return True if all probes passed.
 */
bool hiz_probe_rects();

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
