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

/** @file common.h
 *
 * Common utility functions for the ARB_shader_image_load_store tests.
 */

#ifndef __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_COMMON_H__
#define __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_COMMON_H__

#include "image.h"
#include "grid.h"
#include "piglit-util-gl.h"

/**
 * Report the result of a subtest using a more convenient syntax.
 */
#define subtest(status, guard, result, ...) do {                        \
                enum piglit_result _status = (!(guard) ? PIGLIT_SKIP :  \
                                              (result) ? PIGLIT_PASS :  \
                                              PIGLIT_FAIL);             \
                                                                        \
                piglit_report_subtest_result(_status, __VA_ARGS__);     \
                                                                        \
                if (_status == PIGLIT_FAIL)                             \
                        *status = PIGLIT_FAIL;                          \
        } while (0)

/**
 * Set an integer uniform to the specified value.
 */
bool
set_uniform_int(GLuint prog, const char *name, int value);

/**
 * Accessor for the texture object bound to the specified image unit.
 */
GLuint
get_texture(unsigned unit);

/**
 * Accessor for the buffer object bound to the specified image unit.
 */
GLuint
get_buffer(unsigned unit);

/**
 * Upload \a pixels to an image of the specified format and
 * dimensionality, and bind it to the specified image unit.
 */
bool
upload_image(const struct image_info img, unsigned unit,
             const uint32_t *pixels);

/**
 * Analogous to upload_image(), but in addition it may be used to
 * specify \a num_levels mipmap levels for the same texture at once.
 * Level \a level will be bound to the given image unit.
 */
bool
upload_image_levels(const struct image_info img, unsigned num_levels,
                    unsigned level, unsigned unit,
                    const uint32_t *pixels);

/**
 * Download the image bound to the specified image unit into
 * \a r_pixels.
 */
bool
download_image(const struct image_info img, unsigned unit,
               uint32_t *r_pixels);

/**
 * Analogous to download_image(), but in addition it may be used to
 * download \a num_levels mipmap levels at once from the same image.
 */
bool
download_image_levels(const struct image_info img, unsigned num_levels,
                      unsigned unit, uint32_t *r_pixels);

/**
 * Initialize a two-dimensional array of pixels to the specified
 * constant value.
 */
bool
init_pixels(const struct image_info img, uint32_t *pixels,
            double r, double g, double b, double a);

/**
 * Check that all elements from a two-dimensional array of pixels
 * equal the specified constant value.
 */
bool
check_pixels(const struct image_info img, const uint32_t *pixels,
             double r, double g, double b, double a);

/**
 * Check that two two-dimensional arrays of pixels are equal.
 */
bool
check_pixels_v(const struct image_info img,
               const uint32_t *pixels,
               const uint32_t *expect);

/**
 * Initialize and clear the framebuffer, or an image read-back buffer
 * when using the compute stage.
 */
bool
init_fb(const struct grid_info grid);

/**
 * Download the contents of the framebuffer, or the image read-back
 * buffer when using the compute stage.
 */
bool
download_result(const struct grid_info grid, uint32_t *r_pixels);

#endif
