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
 * \file read-after-clear.c
 *
 * Test that fast color clears of an off-screen buffer work properly
 * when they are followed by various ways of reading from the buffer.
 *
 * There are four sub-tests (selectable by a command line parameter)
 * for each of the following ways of reading from the texture buffer:
 *
 * - sample: read by sampling via a GLSL shader.
 * - read_pixels: read using the glReadPixels() function.
 * - blit: read by blitting from the texture to the windowsystem framebuffer.
 * - copy: read by copying to a second texture using glCopyTexImage2D.
 *
 * In addition, each test can be qualified with "rb" or "tex" to
 * choose whether the off-screen buffer is a texture or a
 * renderbuffer.  Note that the "rb" option is not allowed for the
 * "sample" sub-test.
 *
 * The test operates by creating an off-screen buffer, painting it red
 * using a non-fast-clear technique (rendering a quad using a shader),
 * and then clearing it to green using a fast clear.  Then it reads
 * from the buffer using the technique specified on the command line,
 * to verify that the fast clear data got successfully written to the
 * buffer.
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


static enum subtest_enum {
	SUBTEST_SAMPLE,
	SUBTEST_READ_PIXELS,
	SUBTEST_BLIT,
	SUBTEST_COPY,
} subtest;

static bool use_texture;


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

static GLuint prog_paint_red, prog_sample, tex1, tex2, fb;


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest> <buffer_type>\n"
	       "  where <subtest> is one of the following:\n"
	       "    sample: read by sampling from the cleared buffer\n"
	       "    read_pixels: read using glReadPixels()\n"
	       "    blit: read by blitting from the cleared buffer\n"
	       "    copy: read using glCopyTexImage2D()\n"
	       "  and <buffer_type> is one of the following:\n"
	       "    rb: off-screen buffer is a renderbuffer\n"
	       "    tex: off-screen buffer is a texture\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


static GLuint allocate_texture()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,
		     0 /* level */,
		     GL_RGBA /* internal format */,
		     TEX_WIDTH, TEX_HEIGHT,
		     0 /* border */,
		     GL_RGBA /* format */,
		     GL_BYTE /* type */,
		     NULL /* data */);
	return tex;
}


void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs_paint_red, fs_sample;
	GLenum fb_status;

	/* Parse params */
	if (argc != 3)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "sample") == 0)
		subtest = SUBTEST_SAMPLE;
	else if (strcmp(argv[1], "read_pixels") == 0)
		subtest = SUBTEST_READ_PIXELS;
	else if (strcmp(argv[1], "blit") == 0)
		subtest = SUBTEST_BLIT;
	else if (strcmp(argv[1], "copy") == 0)
		subtest = SUBTEST_COPY;
	else
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[2], "rb") == 0)
		use_texture = false;
	else if (strcmp(argv[2], "tex") == 0)
		use_texture = true;
	else
		print_usage_and_exit(argv[0]);

	/* Detect parameter conflicts */
	if (subtest == SUBTEST_SAMPLE && !use_texture) {
		printf("Subtest 'sample' requires buffer_type 'tex'.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Requirements */
	piglit_require_gl_version(11);
	piglit_require_GLSL_version(110);
	piglit_require_extension("GL_ARB_framebuffer_object");

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

	/* Set up framebuffer */
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	if (use_texture) {
		tex1 = allocate_texture();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, tex1, 0 /* level */);
	} else {
		GLuint rb;
		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
				      TEX_WIDTH, TEX_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, rb);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer status: %s\n",
		       piglit_get_gl_enum_name(fb_status));
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up second texture (used by "copy" test only) */
	if (subtest == SUBTEST_COPY)
		tex2 = allocate_texture();
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static const GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };

	/* Paint the texture red using a shader (not a fast clear). */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glUseProgram(prog_paint_red);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Clear the texture to green; this will be optimized using
	 * a fast color clear if the hardware is capable of it.
	 */
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (subtest) {
	case SUBTEST_SAMPLE:
		/* Sample from the texture and draw to the window
		 * system framebuffer.
		 */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glViewport(0, 0, piglit_width, piglit_height);
		glUseProgram(prog_sample);
		glUniform1i(glGetUniformLocation(prog_sample, "samp"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);
		piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		pass = piglit_probe_rect_rgba(0, 0, piglit_width,
					      piglit_height, green) && pass;
		break;
	case SUBTEST_READ_PIXELS:
		/* Read directly from the texture using
		 * glReadPixels().
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		pass = piglit_probe_rect_rgba(0, 0, TEX_WIDTH, TEX_HEIGHT,
					      green) && pass;
		/* And then clear the window system framebuffer to
		 * black since there is nothing to display in this
		 * subtest.
		 */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		break;
	case SUBTEST_BLIT:
		/* Blit from the texture to the window system
		 * fraembuffer.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, TEX_WIDTH, TEX_HEIGHT,
				  0, 0, piglit_width, piglit_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		pass = piglit_probe_rect_rgba(0, 0, piglit_width,
					      piglit_height, green) && pass;
		break;
	case SUBTEST_COPY:
		/* Copy to a second texture using glCopyTexImage2D(). */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glBindTexture(GL_TEXTURE_2D, tex2);
		glCopyTexImage2D(GL_TEXTURE_2D,
				 0 /* level */,
				 GL_RGBA,
				 0, 0, TEX_WIDTH, TEX_HEIGHT,
				 0 /* border */);
		/* Sample from the second texture and draw to the
		 * window system framebuffer.
		 */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glViewport(0, 0, piglit_width, piglit_height);
		glUseProgram(prog_sample);
		glUniform1i(glGetUniformLocation(prog_sample, "samp"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex2);
		piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		pass = piglit_probe_rect_rgba(0, 0, piglit_width,
					      piglit_height, green) && pass;
		break;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
