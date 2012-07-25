/* Copyright © 2012 Intel Corporation
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

#include "common.h"

/**
 * \file draw-buffers-common.h
 * This file declares common functions which are used by multiple draw buffer
 * test cases.
 */

/* Allocates all the relevant data arrays required in the test */
void allocate_data_arrays(void);

/* Draws a test pattern without sample_alpha_to_coverage and
 * sample_alpha_to_one
 */
void draw_reference_image(bool sample_alpha_to_coverage,
			  bool sample_alpha_to_one);

/* Draws the test pattern with either sample_alpha_to_coverage or
 * sample_alpha_to_one enabled
 */
void draw_test_image(bool sample_alpha_to_coverage,
		     bool sample_alpha_to_one);

/* Frees the previously allocated data arrays */
void free_data_arrays(void);

/* Initilaizes multisample framebuffer object with multiple draw buffers */
void ms_fbo_and_draw_buffers_setup(int samples,
				   int width, int height,
				   int n_attachments,
				   GLenum test_buffers,
				   GLenum color_buffer_zero_format);

/* Probe downsampled FBO (resolve_fbo / resolve_int_fbo) to compare against
 * expected color for each draw buffer.
 */
bool probe_framebuffer_color(void);

/* Probe downsampled FBO (resolve_fbo) to compare against expected depth values
 * for each draw buffer.
 */
bool probe_framebuffer_depth(void);

void shader_compile(bool sample_alpha_to_coverage, bool dual_src_blend);
