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

/** @file grid.h
 *
 * Utility code for running a grid of shader invocations abstracting
 * out the details of the specific shader stage it's run on.
 */

#ifndef __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_GRID_H__
#define __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_GRID_H__

#include "image.h"

struct grid_info {
        /** Bitfield of shader stages present in the pipeline
         * (GL_*_SHADER_BIT). */
        unsigned stages;

        /** Data type used to hold the values that are passed down
         * through the pipeline. */
        const struct image_format_info *format;

        /** Size of the two-dimensional grid. */
        struct image_extent size;
};

/**
 * Construct a grid_info object.
 */
static inline struct grid_info
grid_info(GLenum stage, GLenum format, unsigned w, unsigned h)
{
        const struct grid_info grid = {
                get_image_stage(stage)->bit,
                get_image_format(format),
                { w, h, 1, 1 }
        };

        return grid;
}

static inline struct grid_info
set_grid_size(struct grid_info grid, unsigned x, unsigned y)
{
        const struct image_extent size = { x, y, 1, 1 };
        grid.size = size;
        return grid;
}

/**
 * Construct an image_info structure with the same dimensions and
 * format as the specified grid.
 */
static inline struct image_info
image_info_for_grid(const struct grid_info grid)
{
        const struct image_info img = {
                get_image_target(GL_TEXTURE_2D),
                grid.format, grid.size,
                image_format_epsilon(grid.format)
        };

        return img;
}

/**
 * Concatenate a variable number of strings into a newly allocated
 * buffer.  Note that concat() assumes ownership of the provided
 * arguments and that the argument list must be NULL-terminated.
 */
char *
concat(char *hunk0, ...);

static inline char *
hunk(const char *s)
{
        return strdup(s);
}

/**
 * Generate preprocessor defines containing geometry and data type
 * information for a shader image object.
 */
char *
image_hunk(const struct image_info img, const char *prefix);

/**
 * Generate a shader program containing all the required stages to run
 * the provided shader source from \a grid.  A series of (GLenum, char *)
 * pairs should follow as variadic arguments, where the GLenum
 * argument specifies an additional shader stage (GL_*_SHADER) and the
 * string specifies a fragment of GLSL code to be included in the same
 * shader stage.  Note that generate_program() takes ownership of the
 * provided argument strings.
 *
 * Each fragment should define a GLSL function with prototype
 * "GRID_T op(ivec2 idx, GRID_T x)", where \a idx is the
 * two-dimensional coordinate of a particular shader invocation within
 * the grid and \a x is the result of the last invocation of op() from
 * a previous shader stage at the same grid coordinate.  Zero is
 * passed as argument to the topmost invocation of op() in the chain.
 *
 * The final result from the chain of op() calls is written as
 * fragment color to the framebuffer, or written to the read-back
 * buffer when running a compute shader.
 *
 * The generated program will typically be passed as argument to
 * draw_grid() in order to launch the grid.
 */
GLuint
generate_program(const struct grid_info grid, ...);

/**
 * Launch a grid of shader invocations of the specified size.
 * Depending on the specified shader stages an array of triangles,
 * points or patches will be drawn or a compute grid will be
 * executed.
 */
bool
draw_grid(const struct grid_info grid, GLuint prog);

/**
 * Generate vertex arrays intended to be used to launch a grid of
 * shader invocations using the specified origin (\a x, \a y), spacing
 * (\a dx, \a dy) and dimensions (\a nx, \a ny).  This is done
 * internally by draw_grid(), but it could be useful on its own for
 * applications that require more control.
 */
bool
generate_grid_arrays(GLuint *vao, GLuint *vbo,
                     float x, float y, float dx, float dy,
                     unsigned nx, unsigned ny);

#endif
