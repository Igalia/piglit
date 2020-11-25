/*
 * Copyright © 2010 Intel Corporation
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
 *
 */

/** @file fbo-pbo-readpixels-small.c
 *
 * Tests that PBO blit readpixels on a 2x2 FBO works correctly.  Based
 * on a description of a failure in clutter and figuring out the associated
 * bug.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=25921
 *
 * Added depth and stencil support to test the issue:
 * https://gitlab.freedesktop.org/mesa/mesa/-/issues/3775
 *
 * Authors:
 * \author Eric Anholt <eric@anholt.net>
 * \author Yevhenii Kharchenko <yevhenii.kharchenko@globallogic.com>
 *
 */

#include "piglit-util-gl.h"

const struct piglit_subtest subtests[];
static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.subtests = subtests;

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE |
			       PIGLIT_GL_VISUAL_RGB |
			       PIGLIT_GL_VISUAL_DEPTH |
			       PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

typedef struct test_case
{
	GLenum internal_format;
	GLenum texture_component;
	GLenum component_to_read;
	GLenum texture_type;
	GLenum type_to_read;
	GLenum attachment;
} test_case_t;

const struct test_case test_case_list[] = {
	{
		GL_RGBA,
		GL_RGBA,
		GL_BGRA,
		GL_UNSIGNED_BYTE,
		GL_UNSIGNED_BYTE,
		GL_COLOR_ATTACHMENT0_EXT
	},
	{
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		GL_FLOAT,
		GL_DEPTH_ATTACHMENT_EXT
	},
	{
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		GL_FLOAT,
		GL_DEPTH_ATTACHMENT_EXT
	},
	{
		GL_DEPTH_COMPONENT32F,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		GL_FLOAT,
		GL_DEPTH_ATTACHMENT_EXT
	},
	{
		GL_DEPTH24_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_INT_24_8,
		GL_FLOAT,
		GL_DEPTH_STENCIL_ATTACHMENT
	},
	{
		GL_DEPTH24_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_STENCIL_INDEX,
		GL_UNSIGNED_INT_24_8,
		GL_UNSIGNED_BYTE,
		GL_DEPTH_STENCIL_ATTACHMENT
	},
	{
		GL_DEPTH24_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_DEPTH_STENCIL,
		GL_UNSIGNED_INT_24_8,
		GL_UNSIGNED_INT_24_8,
		GL_DEPTH_STENCIL_ATTACHMENT
	},
	{
		GL_DEPTH32F_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_DEPTH_COMPONENT,
		GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		GL_FLOAT,
		GL_DEPTH_STENCIL_ATTACHMENT
	},
	{
		GL_DEPTH32F_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_STENCIL_INDEX,
		GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		GL_UNSIGNED_BYTE,
		GL_DEPTH_STENCIL_ATTACHMENT
	},
	{
		GL_DEPTH32F_STENCIL8,
		GL_DEPTH_STENCIL,
		GL_DEPTH_STENCIL,
		GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		GL_DEPTH_STENCIL_ATTACHMENT
	}
};

const uint8_t STENCIL_VALUES[4] = {1, 1, 3, 1};
const float DEPTH_VALUES[4] = {0.1f, 0.1f, 0.3f, 0.1f};

static void
make_fbo(GLuint *fbo, GLuint *tex, struct test_case config)
{
	GLenum status;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, 2, 2, 0,
		     config.texture_component, config.texture_type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffersEXT(1, fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  config.attachment,
				  GL_TEXTURE_2D,
				  *tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "framebuffer incomplete (status = 0x%04x)\n",
			status);
		abort();
	}
}


static void
make_pbo(GLuint *pbo)
{
	/* Size for max pixel size of 64bit(GL_DEPTH32F_STENCIL8) */
	const size_t bufferSize = 2 * 2 * 8;

	glGenBuffersARB(1, pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, *pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, bufferSize,
			NULL, GL_STREAM_DRAW_ARB);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

static GLboolean
probe_rgba(int x, int y, const uint8_t *expected, const uint8_t *observed)
{
	if (expected[0] != observed[0] ||
	    expected[1] != observed[1] ||
	    expected[2] != observed[2] ||
	    expected[3] != observed[3]) {
		printf("Probe color at (%i,%i)\n", x, y);
		printf("  Expected: b = %u  g = %u  r = %u  a = %u\n",
			expected[0], expected[1], expected[2], expected[3]);
		printf("  Observed: b = %u  g = %u  r = %u  a = %u\n",
			observed[0], observed[1], observed[2], observed[3]);
		return GL_FALSE;
	} else {
		return GL_TRUE;
	}
}

static GLboolean
probe_byte(int x, int y, const uint8_t *expected, const uint8_t *observed)
{
	if (*expected != *observed) {
		printf("Probe color at (%i,%i)\n", x, y);
		printf("  Expected: %u\n", *expected);
		printf("  Observed: %u\n", *observed);
		return GL_FALSE;
	} else {
		return GL_TRUE;
	}
}


static GLboolean
probe_depth_stencil(int x, int y, struct test_case config,
		    const float *expected_depth,
		    const uint8_t *expected_stencil,
		    const uint8_t *observed)
{
	GLboolean pass = GL_TRUE;

	switch (config.type_to_read) {
		case GL_UNSIGNED_INT_24_8:
		{
			uint32_t depth_as_uint = 0;
			memcpy((uint8_t *)(&depth_as_uint)+1, observed+1, 3);
			float depth = depth_as_uint / (float)UINT32_MAX;

			pass &= (piglit_compare_pixels(x, y,
						       expected_depth,
						       &depth,
						       piglit_tolerance, 1)
						       != 0);

			pass &= probe_byte(x, y,
					   expected_stencil, &observed[0]);
			break;
		}
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
		{
			float depth = *((float*)observed);
			pass &= (piglit_compare_pixels(x, y,
						       expected_depth,
						       &depth,
						       piglit_tolerance, 1)
						       != 0);

			pass &= probe_byte(x, y,
					   expected_stencil, &observed[4]);
			break;
		}
		default:
			return GL_FALSE;
	}
	return pass;
}

GLboolean
draw_and_probe_rgba(struct test_case config)
{
	static uint8_t green[] = {0, 255u, 0, 0};
	static uint8_t blue[]  = {255u, 0, 0, 0};

	GLboolean pass = GL_TRUE;
	uint8_t *addr;

	/* bottom: green.  top: blue. */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(0, 0, 2, 1);
	glColor4f(0.0, 0.0, 1.0, 0.0);
	piglit_draw_rect(0, 1, 2, 1);

	glReadPixels(0, 0, 2, 2,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)0);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

	pass &= probe_rgba(0, 0, green, addr);
	pass &= probe_rgba(1, 0, green, addr + 4);
	pass &= probe_rgba(0, 1, blue, addr + 8);
	pass &= probe_rgba(1, 1, blue, addr + 12);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with X offset. */
	glReadPixels(1, 0, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_rgba(1, 0, green, addr + 4);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with XY offset. */
	glReadPixels(1, 1, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_rgba(1, 1, blue, addr + 4);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	return pass;
}

GLboolean
draw_and_probe_depth(struct test_case config)
{
	const size_t element_size = 4;

	GLboolean pass = GL_TRUE;
	uint8_t *addr;

	/* bottom left: 0.3. other: 0.1. */
	glDepthMask(GL_TRUE);
	glClearDepth(0.1f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 1, 1, 1);
	glClearDepth(0.3f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glReadPixels(0, 0, 2, 2,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)0);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

	pass &= (piglit_compare_pixels(0, 0,
				       &DEPTH_VALUES[0],
				       (float *)addr,
				       piglit_tolerance, 1) != 0);
	pass &= (piglit_compare_pixels(1, 0,
				       &DEPTH_VALUES[1],
				       (float *)(addr + element_size * 1),
				       piglit_tolerance, 1) != 0);
	pass &= (piglit_compare_pixels(0, 1,
				       &DEPTH_VALUES[2],
				       (float *)(addr + element_size * 2),
				       piglit_tolerance, 1) != 0);
	pass &= (piglit_compare_pixels(1, 1,
				       &DEPTH_VALUES[3],
				       (float *)(addr + element_size * 3),
				       piglit_tolerance, 1) != 0);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an X offset. */
	glReadPixels(1, 0, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= (piglit_compare_pixels(1, 0,
				       &DEPTH_VALUES[1],
				       (float *)(addr + element_size * 1),
				       piglit_tolerance, 1) != 0);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an XY offset. */
	glReadPixels(1, 1, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= (piglit_compare_pixels(1, 1,
				       &DEPTH_VALUES[3],
				       (float *)(addr + element_size * 1),
				       piglit_tolerance, 1) != 0);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	return pass;
}

GLboolean
draw_and_probe_stencil(struct test_case config)
{
	const size_t element_size = 1;

	GLboolean pass = GL_TRUE;
	uint8_t *addr;

	/* bottom left: 3. other: 1. */
	glStencilMask(255u);
	glClearStencil(1);
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 1, 1, 1);
	glClearStencil(3);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glReadPixels(0, 0, 2, 2,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)0);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

	pass &= probe_byte(0, 0, &STENCIL_VALUES[0], addr);
	pass &= probe_byte(1, 0, &STENCIL_VALUES[1], addr + element_size * 1);
	pass &= probe_byte(0, 1, &STENCIL_VALUES[2], addr + element_size * 2);
	pass &= probe_byte(1, 1, &STENCIL_VALUES[3], addr + element_size * 3);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an X offset. */
	glReadPixels(1, 0, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_byte(1, 0, &STENCIL_VALUES[1], addr + element_size * 1);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an XY offset. */
	glReadPixels(1, 1, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)4);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_byte(1, 1, &STENCIL_VALUES[3], addr + element_size * 1);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	return pass;
}


GLboolean
draw_and_probe_depth_and_stencil(struct test_case config)
{
	const size_t element_size = (config.type_to_read ==
			       GL_FLOAT_32_UNSIGNED_INT_24_8_REV) ? 8 : 4;

	GLboolean pass = GL_TRUE;
	uint8_t *addr;

	/* bottom left: 3. other: 1. */
	glDepthMask(GL_TRUE);
	glStencilMask(255u);
	glClearDepth(0.1f);
	glClearStencil(1);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 1, 1, 1);
	glClearDepth(0.3f);
	glClearStencil(3);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glReadPixels(0, 0, 2, 2,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)0);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

	pass &= probe_depth_stencil(0, 0, config,
				    &DEPTH_VALUES[0], &STENCIL_VALUES[0],
				    addr);
	pass &= probe_depth_stencil(1, 0, config,
				    &DEPTH_VALUES[1], &STENCIL_VALUES[1],
				    addr + element_size * 1);
	pass &= probe_depth_stencil(0, 1, config,
				    &DEPTH_VALUES[2], &STENCIL_VALUES[2],
				    addr + element_size * 2);
	pass &= probe_depth_stencil(1, 1, config,
				    &DEPTH_VALUES[3], &STENCIL_VALUES[3],
				    addr + element_size * 3);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an X offset. */
	glReadPixels(1, 0, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)element_size);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_depth_stencil(1, 0, config,
				    &DEPTH_VALUES[1], &STENCIL_VALUES[1],
				    addr + element_size * 1);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an XY offset. */
	glReadPixels(1, 1, 1, 1,
		     config.component_to_read, config.type_to_read,
		     (void *)(uintptr_t)element_size);

	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);

	pass &= probe_depth_stencil(1, 1, config,
				    &DEPTH_VALUES[3], &STENCIL_VALUES[3],
				    addr + element_size * 3);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	return pass;
}

enum piglit_result
run_test_case(void *data)
{
	GLboolean pass = GL_TRUE;
	GLuint fbo, tex, pbo;

	const test_case_t config = *((test_case_t *) data);

	make_fbo(&fbo, &tex, config);
	make_pbo(&pbo);

	glViewport(0, 0, 2, 2);

	switch (config.component_to_read ) {
		case GL_BGRA:
			pass = draw_and_probe_rgba(config);
			break;
		case GL_DEPTH_COMPONENT:
			pass = draw_and_probe_depth(config);
			break;
		case GL_STENCIL_INDEX:
			pass = draw_and_probe_stencil(config);
			break;
		case GL_DEPTH_STENCIL:
			pass = draw_and_probe_depth_and_stencil(config);
			break;
		default:
			pass = GL_FALSE;
			break;
	}

	glDeleteBuffersARB(1, &pbo);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);

	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result pass = PIGLIT_SKIP;

	pass = piglit_run_selected_subtests(piglit_config->subtests,
					piglit_config->selected_subtests,
					piglit_config->num_selected_subtests,
					pass);

	return pass;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	glDisable(GL_DITHER);

	piglit_ortho_projection(2, 2, GL_FALSE);
}

const struct piglit_subtest subtests[] = {
	{
		"GL_RGBA",
		"GL_RGBA",
		run_test_case,
		(void *)(&test_case_list[0])
	},
	{
		"GL_DEPTH_COMPONENT16-GL_DEPTH_COMPONENT",
		"GL_DEPTH_COMPONENT16-GL_DEPTH_COMPONENT",
		run_test_case,
		(void *)(&test_case_list[1])
	},
	{
		"GL_DEPTH_COMPONENT24-GL_DEPTH_COMPONENT",
		"GL_DEPTH_COMPONENT24-GL_DEPTH_COMPONENT",
		run_test_case,
		(void *)(&test_case_list[2])
	},
	{
		"GL_DEPTH_COMPONENT32F-GL_DEPTH_COMPONENT",
		"GL_DEPTH_COMPONENT32F-GL_DEPTH_COMPONENT",
		run_test_case,
		(void *)(&test_case_list[3])
	},
	{
		"GL_DEPTH24_STENCIL8-GL_DEPTH_COMPONENT",
		"GL_DEPTH24_STENCIL8-GL_DEPTH_COMPONENT",
		run_test_case,
		(void *)(&test_case_list[4])
	},
	{
		"GL_DEPTH24_STENCIL8-GL_STENCIL_INDEX",
		"GL_DEPTH24_STENCIL8-GL_STENCIL_INDEX",
		run_test_case,
		(void *)(&test_case_list[5])
	},
	{
		"GL_DEPTH24_STENCIL8-GL_DEPTH_STENCIL",
		"GL_DEPTH24_STENCIL8-GL_DEPTH_STENCIL",
		run_test_case,
		(void *)(&test_case_list[6])
	},
	{
		"GL_DEPTH32F_STENCIL8-GL_DEPTH_COMPONENT",
		"GL_DEPTH32F_STENCIL8-GL_DEPTH_COMPONENT",
		run_test_case,
		(void *)(&test_case_list[7])
	},
	{
		"GL_DEPTH32F_STENCIL8-GL_STENCIL_INDEX",
		"GL_DEPTH32F_STENCIL8-GL_STENCIL_INDEX",
		run_test_case,
		(void *)(&test_case_list[8])
	},
	{
		"GL_DEPTH32F_STENCIL8-GL_DEPTH_STENCIL",
		"GL_DEPTH32F_STENCIL8-GL_DEPTH_STENCIL",
		run_test_case,
		(void *)(&test_case_list[9])
	},
	{0}
};
