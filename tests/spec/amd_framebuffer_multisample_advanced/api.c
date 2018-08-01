/*
 * Copyright © 2018 Advanced Micro Devices, Inc.
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
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_es_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void
piglit_fail(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	piglit_report_result(PIGLIT_FAIL);
}

static void
validate_current_renderbuffer(const char *type, int input_samples, int input_storage_samples)
{
	int samples, storage_samples;

	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_STORAGE_SAMPLES_AMD,
				     &storage_samples);
	piglit_check_gl_error(GL_NO_ERROR);

	if (samples != input_samples ||
	    storage_samples != input_storage_samples) {
		piglit_fail("Created %s buffer (samples = %u, storageSamples = %u), got (%u, %u)\n",
			    type, input_samples, input_storage_samples, samples, storage_samples);
	}
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_AMD_framebuffer_multisample_advanced");

	/* Check enums. */
	int num_modes, *modes;
	glGetIntegerv(GL_NUM_SUPPORTED_MULTISAMPLE_MODES_AMD, &num_modes);
	piglit_check_gl_error(GL_NO_ERROR);
	if (num_modes < 1)
		piglit_fail("GL_NUM_SUPPORTED_MULTISAMPLED_MODES_AMD expected > 0, got %u\n", num_modes);

	int max_color_samples, max_color_storage_samples, max_depthstencil_samples;
	glGetIntegerv(GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD, &max_color_samples);
	glGetIntegerv(GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD, &max_color_storage_samples);
	glGetIntegerv(GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD, &max_depthstencil_samples);
	if (max_color_samples < 4 ||
	    max_color_storage_samples < 4 ||
	    max_depthstencil_samples < 4) {
		piglit_fail("GL_MAX_xxx_SAMPLES_AMD expected >= 4, got %u,%u,%u\n",
			    max_color_samples, max_color_storage_samples,
			    max_depthstencil_samples);
	}
	if (max_color_samples < max_color_storage_samples)
		piglit_fail("GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD < GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD\n");

	modes = malloc(num_modes * 3 * 4);
	glGetIntegerv(GL_SUPPORTED_MULTISAMPLE_MODES_AMD, modes);
	piglit_check_gl_error(GL_NO_ERROR);

	bool found_max_color = false, found_max_zs = false;
	for (int i = 0; i < num_modes; i++) {
		if (modes[i*3] < 2 ||
		    modes[i*3+1] < 1 ||
		    modes[i*3+2] < 1 ||
		    modes[i*3] < modes[i*3+1] ||
		    modes[i*3] < modes[i*3+2])
			piglit_fail("GL_SUPPORTED_MULTISAMPLE_MODES_AMD invalid mode %u,%u,%u\n",
				    modes[i*3], modes[i*3+1], modes[i*3+2]);

		if (modes[i*3] == max_color_samples &&
		    modes[i*3+1] == max_color_storage_samples)
			found_max_color = true;

		if (modes[i*3+2] == max_depthstencil_samples)
			found_max_zs = true;
	}
	if (!found_max_color)
		piglit_fail("Mode with GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD and ..._STORAGE_SAMPLES_AMD not listed\n");
	if (!found_max_zs)
		piglit_fail("Modes with GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD not listed");

	GLuint rb;
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);

	/* Check RenderbufferStorage errors. */
	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 1, -1, GL_RGBA8, 64, 64);
	piglit_check_gl_error(GL_INVALID_VALUE);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, max_color_samples + 1,
						    max_color_storage_samples, GL_RGBA8, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, max_color_samples,
						    max_color_storage_samples + 1, GL_RGBA8, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 2, 3, GL_RGBA8, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 2, 3, GL_DEPTH_COMPONENT24, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 2, 3, GL_STENCIL_INDEX8, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 3, 2, GL_DEPTH_COMPONENT24, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);

	glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, 3, 2, GL_STENCIL_INDEX8, 64, 64);
	piglit_check_gl_error(GL_INVALID_OPERATION);
	glDeleteRenderbuffers(1, &rb);

	/* Check that all modes can be allocated and are framebuffer complete. */
	bool tested_zero_samples = false;

	for (int i = 0; i < num_modes; i++) {
		GLuint fb, cb, db, tmp;

		/* Color */
		glGenRenderbuffers(1, &cb);
		glBindRenderbuffer(GL_RENDERBUFFER, cb);
		glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, modes[i*3], modes[i*3+1],
							    GL_RGBA8, 64, 64);
		piglit_check_gl_error(GL_NO_ERROR);
		validate_current_renderbuffer("color", modes[i*3], modes[i*3+1]);

		/* Depth stencil */
		glGenRenderbuffers(1, &db);
		glBindRenderbuffer(GL_RENDERBUFFER, db);
		glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, modes[i*3+2], modes[i*3+2],
							    GL_DEPTH24_STENCIL8, 64, 64);
		piglit_check_gl_error(GL_NO_ERROR);
		validate_current_renderbuffer("Z/S", modes[i*3+2], modes[i*3+2]);

		/* Framebuffer */
		glGenFramebuffers(1, &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, cb);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, db);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			piglit_fail("Incomplete framebuffer for listed mode %u,%u,%u\n",
				    modes[i*3], modes[i*3+1], modes[i*3+2]);
		}

		glDeleteRenderbuffers(1, &cb);
		glDeleteRenderbuffers(1, &db);
		glDeleteFramebuffers(1, &fb);
		piglit_check_gl_error(GL_NO_ERROR);

		/* Quickly test the other functions and check that
		 * GL_RENDERBUFFER_STORAGE_SAMPLES_AMD is correct.
		 */
		glGenRenderbuffers(1, &tmp);
		glBindRenderbuffer(GL_RENDERBUFFER, tmp); /* Bind-to-create. */
		glBindRenderbuffer(GL_RENDERBUFFER, 0);   /* Unbind before glNamed*. */
		glNamedRenderbufferStorageMultisampleAdvancedAMD(tmp, modes[i*3], modes[i*3+1],
				GL_RGBA8, 64, 64);
		glBindRenderbuffer(GL_RENDERBUFFER, tmp);
		piglit_check_gl_error(GL_NO_ERROR);
		validate_current_renderbuffer("color(glNamed..Advanced)", modes[i*3], modes[i*3+1]);
		glDeleteRenderbuffers(1, &tmp);

		if (modes[i*3] == modes[i*3+1]) {
			/* Standard MSAA. */
			unsigned s = modes[i*3];

			if (piglit_is_extension_supported("GL_ARB_direct_state_access")) {
				glGenRenderbuffers(1, &tmp);
				glBindRenderbuffer(GL_RENDERBUFFER, tmp); /* Bind-to-create. */
				glBindRenderbuffer(GL_RENDERBUFFER, 0);   /* Unbind before glNamed*. */
				glNamedRenderbufferStorageMultisample(tmp, s, GL_RGBA8, 64, 64);
				glBindRenderbuffer(GL_RENDERBUFFER, tmp);
				piglit_check_gl_error(GL_NO_ERROR);
				validate_current_renderbuffer("color(glNamed..Multisample)", s, s);
				glDeleteRenderbuffers(1, &tmp);

				if (!tested_zero_samples) {
					glGenRenderbuffers(1, &tmp);
					glBindRenderbuffer(GL_RENDERBUFFER, tmp); /* Bind-to-create. */
					glBindRenderbuffer(GL_RENDERBUFFER, 0);   /* Unbind before glNamed*. */
					glNamedRenderbufferStorage(tmp, GL_RGBA8, 64, 64);
					glBindRenderbuffer(GL_RENDERBUFFER, tmp);
					piglit_check_gl_error(GL_NO_ERROR);
					validate_current_renderbuffer("color(glNamed..Storage)", 0, 0);
					glDeleteRenderbuffers(1, &tmp);
				}
			}

			glGenRenderbuffers(1, &tmp);
			glBindRenderbuffer(GL_RENDERBUFFER, tmp);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, s, GL_RGBA8, 64, 64);
			validate_current_renderbuffer("color(gl..Multisample)", s, s);
			glDeleteRenderbuffers(1, &tmp);

			if (!tested_zero_samples) {
				glGenRenderbuffers(1, &tmp);
				glBindRenderbuffer(GL_RENDERBUFFER, tmp);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);
				piglit_check_gl_error(GL_NO_ERROR);
				validate_current_renderbuffer("color(gl..Storage)", 0, 0);
				glDeleteRenderbuffers(1, &tmp);
			}
			tested_zero_samples = true;
		}
	}

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
