/*
 * Copyright (c) VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
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


#ifndef PIGLIT_MATRIX_H
#define PIGLIT_MATRIX_H


#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


void
piglit_identity_matrix(float mat[16]);

void
piglit_scale_matrix(float mat[16], float sx, float sy, float sz);

void
piglit_translation_matrix(float mat[16], float tx, float ty, float tz);

void
piglit_rotation_matrix(float mat[16], float angle, float x, float y, float z);

void
piglit_ortho_matrix(float mat[16],
		    float left, float right,
		    float bottom, float top,
		    float nearval, float farval);

void
piglit_frustum_matrix(float mat[16],
		      float left, float right,
		      float bottom, float top,
		      float nearval, float farval);

void
piglit_matrix_mul_matrix(float product[16],
                         const float a[16], const float b[16]);

void
piglit_matrix_mul_vector(float out[4],
                         const float mat[16],
                         const float in[4]);

void
piglit_ndc_to_window(float win[3],
                     const float ndc[4],
                     int vp_left, int vp_bottom, int vp_width, int vp_height);

bool
piglit_project_to_window(float win[3],
                         const float obj[4],
                         const float modelview[16],
                         const float projection[16],
                         int vp_left, int vp_bottom,
                         int vp_width, int vp_height);

void
piglit_print_matrix(const float mat[16]);

void
piglit_matrix_inverse(float inv[16], const float m[16]);

void
piglit_matrix_transpose(float out[16], const float m[16]);

#ifdef __cplusplus
} /* end extern "C" */
#endif


#endif /* PIGLIT_MATRIX_H */
