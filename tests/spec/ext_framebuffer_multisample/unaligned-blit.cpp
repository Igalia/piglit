/*
 * Copyright © 2012 Intel Corporation
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
#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

/**
 * \file unaligned-blit.cpp
 *
 * Verify the accuracy of blits involving MSAA buffers when the blit
 * coordinates are not aligned to simple powers of two.
 *
 * This test operates through the use of a sequence of blits that
 * might be called a "scrambling blit": a source image (whose size is
 * not a power of two) is divided up into tiles (whose size is also
 * not a power of two), and these tiles are blitted one at a time from
 * the source to the destination buffer, permuting the order of the
 * tiles in a deterministic way.  The scrambling ensures that we test
 * a wide variety of different offsets and coordinate misalignments.
 *
 * The test performs the following operations: First an unscrambled
 * test image is created in a source buffer, which may or may not be
 * multisampled.  Then a scrambling blit is used to copy it to a
 * destination buffer, which also may or may not be multisampled.
 * Finally, the destination buffer is blitted to the window system
 * framebuffer, using the inverse permutation.  This should result in
 * an unscrambled test image.
 *
 * To verify that the test image is correct, we produce a reference
 * image by repeating the same operation using ordinary unscrambled
 * blits.
 */
const int pattern_size = 245;
const int tile_size = 49;
const int tiles_across = 5;
const int num_tiles = tiles_across * tiles_across;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 2*pattern_size;
	config.window_height = pattern_size;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

const int permutation[num_tiles] = {
	10, 5, 6, 17, 3, 11, 16, 21, 14, 24, 23, 8, 15, 18, 0, 12, 9,
	4, 22, 19, 20, 2, 7, 13, 1
};

const int inverse_permutation[num_tiles] = {
	14, 24, 21, 4, 17, 1, 2, 22, 11, 16, 0, 5, 15, 23, 8, 12, 6,
	3, 13, 19, 20, 7, 18, 10, 9
};

Fbo src_fbo;
Fbo dst_fbo;
TestPattern *test_pattern = NULL;
ManifestProgram *manifest_program = NULL;
GLbitfield buffer_to_test;

void
scrambling_blit(const int *permutation)
{
	for (int i = 0; i < num_tiles; ++i) {
		int src_x = (i % tiles_across) * tile_size;
		int src_y = (i / tiles_across) * tile_size;
		int dst_x = (permutation[i] % tiles_across) * tile_size;
		int dst_y = (permutation[i] / tiles_across) * tile_size;
		glBlitFramebuffer(src_x, src_y,
				  src_x + tile_size, src_y + tile_size,
				  dst_x, dst_y,
				  dst_x + tile_size, dst_y + tile_size,
				  buffer_to_test, GL_NEAREST);
	}
}

void
NORETURN print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <buffer_type> <blit_type>\n"
	       "  where <buffer_type> is one of:\n"
	       "    color\n"
	       "    stencil\n"
	       "    depth\n"
	       "  and <blit_type> is one of:\n"
	       "    msaa\n"
	       "    upsample\n"
	       "    downsample\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int num_samples;
	int src_samples;
	int dst_samples;
	if (argc < 4)
		print_usage_and_exit(argv[0]);

	char *endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	if (strcmp(argv[2], "color") == 0) {
		test_pattern = new Triangles();
		buffer_to_test = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		test_pattern = new DepthSunburst();
		manifest_program = new ManifestDepth();
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
	} else if (strcmp(argv[2], "stencil") == 0) {
		test_pattern = new StencilSunburst();
		manifest_program = new ManifestStencil();
		buffer_to_test = GL_STENCIL_BUFFER_BIT;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[3], "msaa") == 0) {
		src_samples = dst_samples = num_samples;
	} else if (strcmp(argv[3], "upsample") == 0) {
		src_samples = 0;
		dst_samples = num_samples;
	} else if (strcmp(argv[3], "downsample") == 0) {
		src_samples = num_samples;
		dst_samples = 0;
	} else {
		print_usage_and_exit(argv[0]);
	}

	test_pattern->compile();
	if (manifest_program)
		manifest_program->compile();
	src_fbo.setup(FboConfig(src_samples, pattern_size, pattern_size));
	dst_fbo.setup(FboConfig(dst_samples, pattern_size, pattern_size));
}

enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the test pattern in src_fbo. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo.handle);
	src_fbo.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Blit from src_fbo to dst_fbo, scrambling the pattern as we go. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	scrambling_blit(permutation);

	/* Blit from dst_fbo to the left half of the window system
	 * framebuffer, unscrambling as we go.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	scrambling_blit(inverse_permutation);

	/* Blit from src_fbo to dst_fbo with no scrambling. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_size, pattern_size,
			  0, 0, pattern_size, pattern_size,
			  buffer_to_test, GL_NEAREST);

	/* Blit from dst_fbo to the right half of the window system
	 * framebuffer, with no scrambling.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_size, pattern_size,
			  pattern_size, 0, pattern_size*2, pattern_size,
			  buffer_to_test, GL_NEAREST);

	/* If we were testing depth or stencil, manifest the image so
	 * that we can see it.
	 */
	glViewport(0, 0, piglit_width, piglit_height);
	if (manifest_program)
		manifest_program->run();

	/* Check that the left and right halves of the screen match. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						   piglit_height) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
