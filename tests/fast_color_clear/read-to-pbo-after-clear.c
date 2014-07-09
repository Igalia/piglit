/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file read-to-pbo-after-clear.c
 *
 * On i965/gen7+, glReadPixels uses the hardware blitter when reading
 * from the window system buffer to a PBO, provided that no format
 * conversions need to be performed.  This test verifies that fast
 * color clears are properly resolved before this blit occurs.
 *
 * The test operates by painting the window system framebuffer red
 * using a non-fast-clear technique (rendering a quad using a shader),
 * and then clearing it to green using a fast clear.  Then it reads
 * from the window to a PBO using glReadPixels(), and then maps the
 * PBO into CPU memory and verifies that it contains green.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 512
#define TEX_HEIGHT 512

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_width = TEX_WIDTH;
	config.window_height = TEX_HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const char *vs_text =
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"  gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"}\n";

static const char *fs_text_paint_red =
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

static const char *fs_text_sample =
	"uniform sampler2D samp;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = texture2D(samp, gl_TexCoord[0].xy);\n"
	"}\n";

static GLuint prog_paint_red, prog_sample, pbo;


void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs_paint_red, fs_sample;

	/* Requirements */
	piglit_require_gl_version(11);
	piglit_require_GLSL_version(110);
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	/* Compile shaders */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs_paint_red = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						  fs_text_paint_red);
	prog_paint_red = piglit_link_simple_program(vs, fs_paint_red);
	if (!prog_paint_red)
		piglit_report_result(PIGLIT_FAIL);
	fs_sample = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					       fs_text_sample);
	prog_sample = piglit_link_simple_program(vs, fs_sample);
	if (!prog_sample)
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Set up PBO */
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER, 4 * TEX_WIDTH * TEX_HEIGHT, NULL,
		     GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


bool
check_pbo_data(const GLubyte *expected)
{
	int i, j, k;
	const GLubyte *data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < TEX_HEIGHT; i++) {
		for (j = 0; j < TEX_WIDTH; j++) {
			const GLubyte *actual = &data[(i * TEX_WIDTH + j) * 4];
			for (k = 0; k < 4; k++) {
				if (actual[k] != expected[k]) {
					printf("Failure at (%d, %d):\n"
					       "Expected BGRA %d, %d, %d, %d\n"
					       "Got BGRA %d, %d, %d, %d\n",
					       i, j,
					       expected[0], expected[1],
					       expected[2], expected[3],
					       actual[0], actual[1],
					       actual[2], actual[3]);
					glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
					return false;
				}
			}
		}
	}
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static const GLubyte green_ubytes[] = { 0, 255, 0, 255 };

	/* Paint the window red using a shader (not a fast clear). */
	glUseProgram(prog_paint_red);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Clear the window to green; this will be optimized using a
	 * fast color clear if the hardware is capable of it.
	 */
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Read directly from the window into a PBO using
	 * glReadPixels().  Note: we read using GL_UNSIGNED_BYTE and
	 * GL_BGRA since that's the case that causes the i965 driver
	 * to perform the read using a blit.
	 */
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glReadPixels(0, 0, TEX_WIDTH, TEX_HEIGHT, GL_BGRA,
		     GL_UNSIGNED_BYTE, NULL);
	pass = check_pbo_data(green_ubytes) && pass;

	/* Note: piglit_present_results() will force a resolve to
	 * occur, so even if the test has failed, the window might
	 * appear green.  To avoid confusing the user, clear the
	 * window to black before calling piglit_present_results().
	 */
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
