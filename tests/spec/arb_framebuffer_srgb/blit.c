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
 * The various GL 4.x specifications contain a lot of conflicting rules
 * about how blits should be handled when the source or destination buffer
 * is sRGB.
 *
 * Here are the latest rules from GL 4.4 (October 18th, 2013)
 * section 18.3.1 Blitting Pixel Rectangles:
 *
 * (1) When values are taken from the read buffer, if [[FRAMEBUFFER_SRGB
 *     is enabled and]] the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING
 *     for the framebuffer attachment corresponding to the read buffer is
 *     SRGB (see section 9.2.3), the red, green, and blue components are
 *     converted from the non-linear sRGB color space according to
 *     equation 8.14.
 *
 * (2) When values are written to the draw buffers, blit operations
 *     bypass most of the fragment pipeline. The only fragment
 *     operations which affect a blit are the pixel ownership test,
 *     the scissor test, and sRGB conversion (see section
 *     17.3.9). Color, depth, and stencil masks (see section 17.4.2)
 *     are ignored.
 *
 * And from section 17.3.9 sRGB Conversion:
 *
 * (3) If FRAMEBUFFER_SRGB is enabled and the value of
 *     FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer
 *     attachment corresponding to the destination buffer is SRGB1
 *     (see section 9.2.3), the R, G, and B values after blending are
 *     converted into the non-linear sRGB color space by computing
 *     ... [formula follows] ... If FRAMEBUFFER_SRGB is disabled or
 *     the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING is not SRGB,
 *     then ... [no conversion is applied].
 *
 * Rules differ in other specifications:
 *
 * -------------------------------------------------------------------
 *
 * ES 3.0 contains identical rules, however, ES has no FRAMEBUFFER_SRGB
 * setting.  References to that are deleted, making encode and decode
 * happen regardless.
 *
 * -------------------------------------------------------------------
 *
 * The GL 4.3 revision from February 14th, 2013 deletes the bracketed
 * text in paragraph (1), which appears to indicate that sRGB decode
 * should happen regardless of the GL_FRAMEBUFFER_SRGB setting.
 *
 * This forces decode, but allows encode or no encode.  This makes it
 * impossible to do blits in a linear colorspace, which is not ideal.
 *
 * I believe this was an oversight: it looks like Khronos imported
 * paragraph (1) from ES 3.x but neglected to add a FRAMEBUFFER_SRGB
 * interaction on decode.
 *
 * -------------------------------------------------------------------
 *
 * The older GL 4.3 revision from August 6th, 2012 contains that
 * same decode-always version of paragraph (1), but also contains
 * another paragraph immediately after:
 *
 * (4) When values are taken from the read buffer, no linearization is
 *     performed even if the format of the buffer is SRGB.
 *
 * These are irreconcilable: the first says that linearization should
 * happen when reading from SRGB buffers, while the second says that
 * it shouldn't.  These rules are not implementable, which is probably
 * why they changed in a point revision.
 *
 * -------------------------------------------------------------------
 *
 * GL 4.2 omits paragraph (1) entirely but contains (4), suggesting that
 * decode should never happen, but encode might.
 *
 * -------------------------------------------------------------------
 *
 * GL 4.1 and earlier specifications omits both paragraphs (1) and (4),
 * and contain an alternate version of paragraph (2):
 *
 * (2b) Blit operations bypass the fragment pipeline.  The only fragment
 *      operations which affect a blit are the pixel ownership test and
 *      the scissor test.
 *
 * Notably missing is sRGB conversion.
 *
 * This suggests that neither encode nor decode should happen, regardless
 * of the FRAMEBUFFER_SRGB setting.  These are the traditional GL rules.
 *
 * -------------------------------------------------------------------
 *
 * To summarize the rule differences:
 *
 *      Specification   Decoding   Encoding
 *      ES 3.x          Yes        Yes
 *      GL 4.1          No         No
 *      GL 4.2          No         Optional
 *      GL 4.3 2012     Yes & No   Optional
 *      GL 4.3 2013     Yes        Optional
 *      GL 4.4          Optional   Optional
 *
 * -------------------------------------------------------------------
 *
 * When this test was written in 2012, the author surveyed the nVidia
 * and AMD drivers of the time.  They appeared to follow the simpler rule
 * that blits preserved the underlying binary representation of the pixels,
 * regardless of whether the format was sRGB and regardless of the setting
 * of FRAMEBUFFER_SRGB.  Left 4 Dead 2 appeared to rely on this behavior
 * at the time, but no longer does as of 2016.
 *
 * Unlike OpenGL, the ES 3.x rules have always been clear: always decode
 * and encode.  Both dEQP and WebGL conformance tests require this.
 *
 * The new GL 4.4 rules are flexible: if GL_FRAMEBUFFER_SRGB is disabled
 * (the default setting), BlitFramebuffer will neither decode nor encode
 * (the traditional GL rules).  If it's enabled, then it follows the ES 3
 * rules (both decode and encode).  This isn't entirely compatible, but it
 * seems like the best solution possible, and the one we should implement.
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
const float src_clear_col = 128.0 / 255.0;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* Test parameters */
static bool use_textures;
static GLenum src_format;
static GLenum dst_format;
static GLsizei src_samples;
static GLsizei dst_samples;
static bool scaled_blit;
static bool enable_srgb_framebuffer;
static bool src_fill_mode_clear;

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
	       "          <src_fill_mode>\n"
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
	       "    disabled\n"
	       "  where src_fill_mode is one of:\n"
	       "    clear\n"
	       "    render\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLint max_samples;

	if (argc != 6) {
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
		enable_srgb_framebuffer = false;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[5], "clear") == 0) {
		src_fill_mode_clear = true;
	} else if (strcmp(argv[5], "render") == 0) {
		src_fill_mode_clear = false;
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

/**
 * Implements GL 4.4 equation 8.14.
 */
static float
srgb_to_linear(float c_s)
{
	return c_s <= 0.04045 ? c_s / 12.92f
			      : powf((c_s + 0.055f) / 1.055f, 2.4f);
}

/**
 * Implements GL 4.4 equation 17.1.
 */
static float
linear_to_srgb(float c_l)
{
	if (c_l <= 0.0f)
		return 0.0f;
	else if (c_l < 0.0031308f)
		return 12.92f * c_l;
	else if (c_l < 1.0f)
		return 1.055f * powf(c_l, 0.41666f) - 0.055f;
	return 1.0f;
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
				float val = src_fill_mode_clear ?
					    src_clear_col : x / 255.0;
				if (component < 3 && enable_srgb_framebuffer) {
					if (src_format == GL_SRGB8_ALPHA8)
						val = srgb_to_linear(val);
					if (dst_format == GL_SRGB8_ALPHA8)
						val = linear_to_srgb(val);
				}

				expected_data[(y * PATTERN_WIDTH + x)
					      * 4 + component] = val;
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

	/* Draw the source image */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo);
	if (src_fill_mode_clear) {
		/* This case is of particular interest to Intel GPUs. */
		glClearColor(src_clear_col, src_clear_col,
			     src_clear_col, src_clear_col);
		glClear(GL_COLOR_BUFFER_BIT);
	} else {
		glViewport(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT);
		piglit_draw_rect(-1, -1, 2, 2);
	}

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
