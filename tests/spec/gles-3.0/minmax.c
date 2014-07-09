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

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	/* If the value's type is listed as Z in a spec table, then consider
	 * its type to be a signed int (that is, GLint or GLint64). If the
	 * value's type is listed as Z^+, then consider its type to be
	 * unsigned (that is, GLuint or GLuint64).
	 */

	GLuint v_blocks;
	GLuint v_uniforms;
	GLuint f_blocks;
	GLuint f_uniforms;
	GLuint64 blocksize;

	piglit_print_minmax_header();

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,     (GLint*) &v_blocks);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, (GLint*) &v_uniforms);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,   (GLint*) &f_blocks);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, (GLint*) &f_uniforms);
	glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, (GLint64*) &blocksize);

	/* Table 6.27 */
	piglit_test_min_uint64(GL_MAX_ELEMENT_INDEX, (1 << 24) - 1);
	piglit_test_min_uint(GL_SUBPIXEL_BITS, 4);
	piglit_test_min_uint(GL_MAX_3D_TEXTURE_SIZE, 256);
	piglit_test_min_uint(GL_MAX_TEXTURE_SIZE, 2048);
	piglit_test_min_uint(GL_MAX_ARRAY_TEXTURE_LAYERS, 256);
	piglit_test_min_float(GL_MAX_TEXTURE_LOD_BIAS, 2.0);
	piglit_test_min_uint(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 2048);
	piglit_test_min_uint(GL_MAX_RENDERBUFFER_SIZE, 2048);
	piglit_test_min_uint(GL_MAX_DRAW_BUFFERS, 4);
	piglit_test_min_uint(GL_MAX_COLOR_ATTACHMENTS, 4);
	piglit_test_min_viewport_dimensions();
	piglit_test_range_float(GL_ALIASED_POINT_SIZE_RANGE, 1, 1);
	piglit_test_range_float(GL_ALIASED_LINE_WIDTH_RANGE, 1, 1);

	/* Table 6.28 */
	piglit_test_min_uint(GL_NUM_COMPRESSED_TEXTURE_FORMATS, 10);
	piglit_test_min_uint(GL_NUM_PROGRAM_BINARY_FORMATS, 0);
	piglit_test_min_uint(GL_NUM_SHADER_BINARY_FORMATS, 0);
	piglit_test_min_uint(GL_MAX_SERVER_WAIT_TIMEOUT, 0);

	/* Table 6.29 */
	piglit_test_min_int(GL_MAJOR_VERSION, 3);

	/* Table 6.30 */
	piglit_test_min_uint(GL_MAX_VERTEX_ATTRIBS, 16);
	piglit_test_min_uint(GL_MAX_VERTEX_UNIFORM_COMPONENTS, 1024);
	piglit_test_min_uint(GL_MAX_VERTEX_UNIFORM_VECTORS, 256);
	piglit_test_min_uint(GL_MAX_VERTEX_UNIFORM_BLOCKS, 12);
	piglit_test_min_uint(GL_MAX_VERTEX_UNIFORM_COMPONENTS, 64);
	piglit_test_min_uint(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 16);

	/* Table 6.31 */
	piglit_test_min_uint(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 896);
	piglit_test_min_uint(GL_MAX_FRAGMENT_UNIFORM_VECTORS, 224);
	piglit_test_min_uint(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, 12);
	piglit_test_min_uint(GL_MAX_FRAGMENT_INPUT_COMPONENTS, 60);
	piglit_test_min_uint(GL_MAX_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_max_int(GL_MIN_PROGRAM_TEXEL_OFFSET, -8);
	piglit_test_min_int(GL_MAX_PROGRAM_TEXEL_OFFSET, 7);

	/* Table 6.32 */
	piglit_test_min_uint(GL_MAX_UNIFORM_BUFFER_BINDINGS, 24);
	piglit_test_min_uint64(GL_MAX_UNIFORM_BLOCK_SIZE, 16384);
	piglit_test_min_uint(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 1);
	piglit_test_min_uint(GL_MAX_COMBINED_UNIFORM_BLOCKS, 24);
	piglit_test_min_uint64(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, v_blocks * blocksize / 4 + v_uniforms);
	piglit_test_min_uint64(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, f_blocks * blocksize / 4 + f_uniforms);
	piglit_test_min_uint64(GL_MAX_VARYING_COMPONENTS, 60);
	piglit_test_min_uint64(GL_MAX_VARYING_VECTORS, 15);
	piglit_test_min_uint64(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 32);

	/* Table 6.33 */
	piglit_test_min_uint(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, 64);
	piglit_test_min_uint(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, 4);
	piglit_test_min_uint(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, 4);

	/* Table 6.34 */
	piglit_test_min_uint(GL_SAMPLE_BUFFERS, 0);
	piglit_test_min_uint(GL_SAMPLES, 0);
	piglit_test_min_uint(GL_MAX_SAMPLES, 4);


	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
