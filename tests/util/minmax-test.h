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

extern bool piglit_minmax_pass;

void piglit_print_minmax_header(void);
void piglit_test_min_int(GLenum token, GLint val);
void piglit_test_max_int(GLenum token, GLint val);
void piglit_test_min_int_v(GLenum token, GLuint index, GLint val);
void piglit_test_max_int_v(GLenum token, GLuint index, GLint val);
void piglit_test_min_uint(GLenum token, GLuint val);
void piglit_test_max_uint(GLenum token, GLuint val);
void piglit_test_min_float(GLenum token, GLfloat val);
void piglit_test_max_float(GLenum token, GLfloat val);
void piglit_test_range_float(GLenum token, GLfloat low, GLfloat high);
void piglit_test_min_viewport_dimensions();

void piglit_test_min_int64(GLenum token, GLint64 min);
void piglit_test_max_int64(GLenum token, GLint64 min);
void piglit_test_min_uint64(GLenum token, GLuint64 min);
void piglit_test_max_uint64(GLenum token, GLuint64 min);

void piglit_test_oq_bits(void);
void piglit_test_tf_bits(GLenum target);
