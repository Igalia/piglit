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

/** \file blit.c
 *
 * Test the sRGB behaviour of blits.
 *
 * The GL 4.3 spec is contradictory about how blits should be handled
 * when the source or destination buffer is sRGB.  From section 18.3.1
 * Blitting Pixel Rectangles:
 *
 * (1) When values are taken from the read buffer, if the value of
 *     FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer
 *     attachment corresponding to the read buffer is SRGB (see
 *     section 9.2.3), the red, green, and blue components are
 *     converted from the non-linear sRGB color space according to
 *     equation 8.14.
 *
 * (2) When values are taken from the read buffer, no linearization is
 *     performed even if the format of the buffer is SRGB.
 *
 * (3) When values are written to the draw buffers, blit operations
 *     bypass most of the fragment pipeline. The only fragment
 *     operations which affect a blit are the pixel ownership test,
 *     the scissor test, and sRGB conversion (see section
 *     17.3.9). Color, depth, and stencil masks (see section 17.4.2)
 *     are ignored.
 *
 * (4) If SAMPLE_BUFFERS for either the read framebuffer or draw
 *     framebuffer is greater than zero, no copy is performed and an
 *     INVALID_OPERATION error is generated if the dimensions of the
 *     source and destination rectangles provided to BlitFramebuffer
 *     are not identical, or if the formats of the read and draw
 *     framebuffers are not identical.
 *
 * And from section 17.3.9 sRGB Conversion:
 *
 * (5) If FRAMEBUFFER_SRGB is enabled and the value of
 *     FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer
 *     attachment corresponding to the destination buffer is SRGB1
 *     (see section 9.2.3), the R, G, and B values after blending are
 *     converted into the non-linear sRGB color space by computing
 *     ... [formula follows] ... If FRAMEBUFFER_SRGB is disabled or
 *     the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING is not SRGB,
 *     then ... [no conversion is applied].
 *
 * Paragraphs (1) and (2) seem irreconcilable: the first says that
 * linearization should happen when reading from SRGB buffers, the
 * second says that it shouldn't.
 *
 * The remaining paragraphs are self-consistent, however they aren't
 * consistent with the observed behaviour of existing drivers (notably
 * nVidia and ATI drivers).  Existing drivers seem to follow the much
 * simpler rule that blits preserve the underlying binary
 * representation of the pixels, regardless of whether the format is
 * sRGB and regardless of the setting of FRAMEBUFFER_SRGB.
 * Furthermore, sRGB and non-sRGB formats are considered "identical"
 * for the purposes of paragraph (4).  Existing games seem to rely on
 * this behaviour.
 *
 * This test verifies that blitting is permitted, and preserves the
 * underlying binary representation of the pixels, under any specified
 * combination of the following circumstances:
 *
 * - Using framebuffers backed by textures vs renderbuffers.
 * - Blitting from sRGB vs linear, and to sRGB vs linear.
 * - Doing a 1:1 blit from a single-sampled vs MSAA buffer, and to a
 *   single-sampled vs MSAA buffer, or doing a scaled blit between
 *   two single-sampled buffers.
 * - With FRAMEBUFFER_SRGB enabled vs disabled.
 *
 * The combination to test is selected using command-line parameters.
 *
 * The test operates by rendering an image to a source framebuffer
 * where each pixel's 8-bit color value is equal to its X coordinate.
 * Then it blits this image to a destination framebuffer, and checks
 * (using glReadPixels) that each pixel's 8-bit color value is still
 * equal to its X coordinate.
 *
 * Since glReadPixels cannot be used directly on MSAA buffers, an
 * additional resolve blit is added when necessary, to convert the
 * image to single-sampled before reading the pixel values.
 *
 * Since the pixels in the test image depend only on the X coordinate,
 * it is easy to test proper sRGB performance of scaled blits: we
 * simply make the source rectangle one pixel high, so that the blit
 * requires scaling.  Note that the purpose of this test is to verify
 * that blits exhibit correct sRGB behaviour, not to verify that
 * scaling is performed correctly, so it is not necessary for us to
 * exhaustively test a wide variety of scaling behaviours.
 */

#include "piglit-util-gl.h"

const int PATTERN_WIDTH = 256;
const int PATTERN_HEIGHT = 64;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

/* Test parameters */
static bool use_textures;
static GLenum src_format;
static GLenum dst_format;
static GLsizei src_samples;
static GLsizei dst_samples;
static bool scaled_blit;
static bool enable_srgb_framebuffer;

/* GL objects */
static GLuint src_fbo;
static GLuint dst_fbo;
static GLuint resolve_fbo;
static GLint prog;

static char *vs_text =
	"#version 120\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"}\n";

static char *fs_text =
	"#version 120\n"
	"void main()\n"
	"{\n"
	"  float x = gl_FragCoord.x;\n"
	"  gl_FragColor = vec4((x - 0.5) / 255.0);\n"
	"}\n";

static GLuint
setup_fbo(GLenum internalformat, GLsizei num_samples)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	if (use_textures && num_samples == 0) {
		GLuint tex;
		const GLint level = 0;
		const GLint border = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, level,
			     internalformat, PATTERN_WIDTH, PATTERN_HEIGHT,
			     border, GL_RGBA, GL_BYTE, NULL);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, tex, level);
	} else {
		GLuint rb;
		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, num_samples,
						 internalformat, PATTERN_WIDTH,
						 PATTERN_HEIGHT);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, rb);
	}
	return fbo;
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <backing_type> <sRGB_types> <blit_type>\n"
	       "          <framebuffer_srgb_setting>\n"
	       "  where <backing_type> is one of:\n"
	       "    texture (ignored for multisampled framebuffers)\n"
	       "    renderbuffer\n"
	       "  where <sRGB_types> is one of:\n"
	       "    linear (both buffers linear)\n"
	       "    srgb (both buffers sRGB)\n"
	       "    linear_to_srgb\n"
	       "    srgb_to_linear\n"
	       "  where <blit_type> is one of:\n"
	       "    single_sampled\n"
	       "    upsample\n"
	       "    downsample\n"
	       "    msaa\n"
	       "    scaled\n"
	       "  where framebuffer_srgb_setting is one of:\n"
	       "    enabled\n"
	       "    disabled\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLint max_samples;

	if (argc != 5) {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[1], "texture") == 0) {
		use_textures = true;
	} else if (strcmp(argv[1], "renderbuffer") == 0) {
		use_textures = false;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[2], "linear") == 0) {
		src_format = GL_RGBA;
		dst_format = GL_RGBA;
	} else if (strcmp(argv[2], "srgb") == 0) {
		src_format = GL_SRGB8_ALPHA8;
		dst_format = GL_SRGB8_ALPHA8;
	} else if (strcmp(argv[2], "linear_to_srgb") == 0) {
		src_format = GL_RGBA;
		dst_format = GL_SRGB8_ALPHA8;
	} else if (strcmp(argv[2], "srgb_to_linear") == 0) {
		src_format = GL_SRGB8_ALPHA8;
		dst_format = GL_RGBA;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[3], "single_sampled") == 0) {
		src_samples = 0;
		dst_samples = 0;
		scaled_blit = false;
	} else if (strcmp(argv[3], "upsample") == 0) {
		src_samples = 0;
		dst_samples = 1; /* selects minimum available sample count */
		scaled_blit = false;
	} else if (strcmp(argv[3], "downsample") == 0) {
		src_samples = 1;
		dst_samples = 0;
		scaled_blit = false;
	} else if (strcmp(argv[3], "msaa") == 0) {
		src_samples = 1;
		dst_samples = 1;
		scaled_blit = false;
	} else if (strcmp(argv[3], "scaled") == 0) {
		src_samples = 0;
		dst_samples = 0;
		scaled_blit = true;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[4], "enabled") == 0) {
		enable_srgb_framebuffer = true;
	} else if (strcmp(argv[4], "disabled") == 0) {
		enable_srgb_framebuffer = true;
	} else {
		print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_framebuffer_sRGB");

	/* skip the test if we don't support multisampling */
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (src_samples > max_samples ||
	    dst_samples > max_samples) {
		piglit_report_result(PIGLIT_SKIP);
	}

	prog = piglit_build_simple_program(vs_text, fs_text);

	src_fbo = setup_fbo(src_format, src_samples);
	dst_fbo = setup_fbo(dst_format, dst_samples);
	if (dst_samples != 0)
		resolve_fbo = setup_fbo(dst_format, 0);
	else
		resolve_fbo = 0;
}

static bool
analyze_image(GLuint fbo)
{
	GLfloat *expected_data = malloc(PATTERN_WIDTH * PATTERN_HEIGHT * 4 *
					sizeof(GLfloat));
	unsigned x, y, component;
	bool pass;

	for (y = 0; y < PATTERN_HEIGHT; ++y) {
		for (x = 0; x < PATTERN_WIDTH; ++x) {
			for (component = 0; component < 4; ++component) {
				expected_data[(y * PATTERN_WIDTH + x)
					      * 4 + component] = x / 255.0;
			}
		}
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	pass = piglit_probe_image_rgba(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT,
				       expected_data);
	free(expected_data);
	return pass;
}

enum piglit_result
piglit_display()
{
	bool pass;

	glUseProgram(prog);
	glDisable(GL_FRAMEBUFFER_SRGB);

	/* Clear buffers */
	if (resolve_fbo != 0) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw the source image */
	glViewport(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Do the blit */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);
	if (enable_srgb_framebuffer)
		glEnable(GL_FRAMEBUFFER_SRGB);
	glBlitFramebuffer(0, 0, PATTERN_WIDTH,
			  scaled_blit ? 1 : PATTERN_HEIGHT,
			  0, 0, PATTERN_WIDTH, PATTERN_HEIGHT,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glDisable(GL_FRAMEBUFFER_SRGB);

	/* If necessary, do a resolve blit */
	if (resolve_fbo != 0) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo);
		glBlitFramebuffer(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT,
				  0, 0, PATTERN_WIDTH, PATTERN_HEIGHT,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		pass = analyze_image(resolve_fbo);
	} else {
		pass = analyze_image(dst_fbo);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
