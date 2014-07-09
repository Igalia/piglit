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

/**
 * \file minmax.c
 * Verify that minimum value requirements for implementation limits are
 * satisfied.
 */

#include <limits.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/* These are all the formats that are required to be color-renderable by the
 * OpenGL 3.0 spec.
 */
static const GLenum valid_formats[] = {
	GL_RGBA32F,
	GL_RGBA16,
	GL_RGBA16F,
	GL_RGBA8,
	GL_SRGB8_ALPHA8,
	GL_R11F_G11F_B10F,
	GL_RG32F,
	GL_RG16,
	GL_RG16F,
	GL_RG8,
	GL_R32F,
	GL_R16,
	GL_R16F,
	GL_R8,
	GL_ALPHA8,
};

static const GLenum valid_integer_formats[] = {
	GL_RGBA32I,
	GL_RGBA32UI,
	GL_RGBA16I,
	GL_RGBA16UI,
	GL_RGBA8I,
	GL_RGBA8UI,
	GL_RG32I,
	GL_RG32UI,
	GL_RG16I,
	GL_RG16UI,
	GL_RG8I,
	GL_RG8UI,
	GL_R32I,
	GL_R32UI,
	GL_R16I,
	GL_R16UI,
	GL_R8I,
	GL_R8UI,
};

static const GLenum valid_depth_formats[] = {
	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
	GL_DEPTH_COMPONENT32F,
};

static const GLenum valid_targets_with_tms[] = {
	GL_RENDERBUFFER,
	GL_TEXTURE_2D_MULTISAMPLE,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
try(GLenum target, GLenum format, GLint max_samples,
    const char *max_samples_name)
{
	bool pass = true;
	GLint *buffer;
	int buffer_size_in_elements = 0;
	size_t buffer_size_in_bytes;
	unsigned i;
	GLint previous;

	glGetInternalformativ(target,
			      format,
			      GL_NUM_SAMPLE_COUNTS,
			      1, &buffer_size_in_elements);
	pass = piglit_check_gl_error(0)
		&& pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "Add new table 6.X Internalformat-specific Implementation
	 *     Dependent Values after 6.52"
	 *
	 *                                                       Minimum
	 *     Get Value         Type    Get Command              Value
	 *     ---------         ----    -----------              -------
	 *     SAMPLES           0*xZ+   GetInternalformativ       fn1
	 *     NUM_SAMPLE_COUNTS Z+      GetInternalformativ       1
	 *
	 *     fn1: see section 6.X."
	 */
	if (buffer_size_in_elements < 1) {
		fprintf(stderr,
			"GL_NUM_SAMPLE_COUNTS is %d for %s/%s\n",
			buffer_size_in_elements,
			piglit_get_gl_enum_name(target),
			piglit_get_gl_enum_name(format));
		return false;
	}

	buffer_size_in_bytes = buffer_size_in_elements * sizeof(GLint);
	buffer = malloc(buffer_size_in_bytes);

	/* Try GL_SAMPLES
	 */
	glGetInternalformativ(target,
			      format,
			      GL_SAMPLES,
			      buffer_size_in_elements,
			      buffer);
	pass = piglit_check_gl_error(0)
		&& pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "- SAMPLES: The sample counts supported for this <format> and
	 *        <target> are written into <params>, in descending
	 *        order. Only positive values are returned."
	 *
	 * We take "positive" to mean greater than zero.  Zero isn't a valid
	 * sample count for multisampling.  It's the special value used to
	 * request non-multisampling.
	 */
	previous = INT_MAX;
	for (i = 0; i < buffer_size_in_elements; i++) {
		if (buffer[i] <= 0) {
			fprintf(stderr,
				"Invalid sample count [%u] = %d returned "
				"for %s/%s\n",
				i, buffer[i],
				piglit_get_gl_enum_name(target),
				piglit_get_gl_enum_name(format));
			pass = false;
		}

		if (previous == buffer[i]) {
			fprintf(stderr,
				"Duplicate values [%u] = [%u] = %d returned "
				"for %s/%s\n",
				i - 1, i, buffer[i],
				piglit_get_gl_enum_name(target),
				piglit_get_gl_enum_name(format));
			pass = false;
		}

		if (previous < buffer[i]) {
			fprintf(stderr,
				"Values not in descending order "
				"([%u] = %d) < ([%u] = %d) returned "
				"for %s/%s\n",
				i - 1, previous,
				i, buffer[i],
				piglit_get_gl_enum_name(target),
				piglit_get_gl_enum_name(format));
			pass = false;
		}

		previous = buffer[i];
	}

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "The maximum value in SAMPLES is guaranteed to be at least the
	 *     lowest of the following:
	 *
	 *       - The value of GetIntegerv(MAX_INTEGER_SAMPLES), if
	 *         <internalformat> is a signed or unsigned integer format.
	 *       - The value of GetIntegerv(MAX_DEPTH_TEXTURE_SAMPLES), if
	 *         <internalformat> is a depth/stencil-renderable format and
	 *         <target> is TEXTURE_2D_MULTISAMPLE or
	 *         TEXTURE_2D_MULTISAMPLE_ARRAY.
	 *       - The value of GetIntegerv(MAX_COLOR_TEXTURE_SAMPLES), if
	 *         <internalformat> is a color-renderable format and <target>
	 *         is TEXTURE_2D_MULTISAMPLE or TEXTURE_2D_MULTISAMPLE_ARRAY.
	 *       - The value of GetIntegerv(MAX_SAMPLES)."
	 *
	 * Separate tests will verify the values for GL_MAX_*_SAMPLES.
	 */
	if (buffer[0] < max_samples) {
		fprintf(stderr,
			"GL_SAMPLES (%d) smaller than %s (%d) "
			"for %s/%s\n",
			buffer[0],
			max_samples_name,
			max_samples,
			piglit_get_gl_enum_name(target),
			piglit_get_gl_enum_name(format));
		pass = false;
	}

	free(buffer);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned i;
	const bool tms_supported =
		piglit_is_extension_supported("GL_ARB_texture_multisample");
	GLint max_samples;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query");

	/* Need GL 3 or extensions to support the valid_formats[] above */
        if (piglit_get_gl_version() < 30) {
		piglit_require_extension("GL_ARB_texture_rg");
		piglit_require_extension("GL_ARB_texture_float");
	}

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	for (i = 0; i < ARRAY_SIZE(valid_formats); i++) {
		pass = try(GL_RENDERBUFFER,
			   valid_formats[i],
			   max_samples,
			   "GL_MAX_SAMPLES")
			&& pass;
	}

	if (!tms_supported) {
		for (i = 0; i < ARRAY_SIZE(valid_depth_formats); i++) {
			pass = try(GL_RENDERBUFFER,
				   valid_depth_formats[i],
				   max_samples,
				   "GL_MAX_SAMPLES")
				&& pass;
		}

		/* The OpenGL 3.1 spec says:
		 *
		 *     "The error INVALID_OPERATION may be generated if
		 *     internalformat is a signed or unsigned integer format,
		 *     samples is greater than one, and the implementation
		 *     does not support multisampled integer renderbuffers
		 *     (see “Required Renderbuffer Formats” below)."
		 *
		 * In OpenGL 3.2 or ARB_texture_multisample the query
		 * GL_MAX_INTEGER_SAMPLES is used to determine the maximum
		 * number of samples for integer buffers.  This is checked in
		 * the other code path.
		 */
		for (i = 0; i < ARRAY_SIZE(valid_integer_formats); i++) {
			pass = try(GL_RENDERBUFFER,
				   valid_integer_formats[i],
				   1,
				   "one")
				&& pass;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(valid_targets_with_tms); i++) {
			const char *max_samples_name;
			unsigned j;

			if (valid_targets_with_tms[i] == GL_RENDERBUFFER) {
				glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
				max_samples_name = "GL_MAX_SAMPLES";
			} else {
				glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES,
					      &max_samples);
				max_samples_name = "GL_MAX_COLOR_TEXTURE_SAMPLES";
			}

			for (j = 0; j < ARRAY_SIZE(valid_formats); j++) {
				pass = try(valid_targets_with_tms[i],
					   valid_formats[j],
					   max_samples,
					   max_samples_name)
					&& pass;
			}

			if (valid_targets_with_tms[i] == GL_RENDERBUFFER) {
				glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
				max_samples_name = "GL_MAX_SAMPLES";
			} else {
				glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES,
					      &max_samples);
				max_samples_name = "GL_MAX_DEPTH_TEXTURE_SAMPLES";
			}

			for (j = 0; j < ARRAY_SIZE(valid_depth_formats); j++) {
				pass = try(valid_targets_with_tms[i],
					   valid_depth_formats[j],
					   max_samples,
					   max_samples_name)
					&& pass;
			}

			glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &max_samples);
			max_samples_name = "GL_MAX_INTEGER_SAMPLES";

			for (j = 0; j < ARRAY_SIZE(valid_integer_formats); j++) {
				pass = try(valid_targets_with_tms[i],
					   valid_integer_formats[j],
					   max_samples,
					   max_samples_name)
					&& pass;
			}
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
