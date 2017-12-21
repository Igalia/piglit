/*
 * Copyright © 2011 Intel Corporation
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
 * Attach a GL_TEXTURE_2D to the GL_DEPTH_STENCIL_ATTACHMENT point with
 * glFramebufferTexture2D(), then verify with
 * glGetFramebufferAttachmentParameteriv() that all three of
 * GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, and GL_DEPTH_STENCIL_ATTACHMENT
 * point to the texture.
 */

#include <stdio.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

bool
check_attachment(GLenum attach, GLint expect_name, GLenum expect_cube_map_face)
{
	GLint actual_type;
	GLint actual_name;
	GLint actual_cube_map_face;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      attach,
				              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
				              &actual_type);

	if (actual_type != GL_TEXTURE) {
		fprintf(stderr,
			"error: expected GL_TEXTURE for %s attachment type, but found %s\n",
			piglit_get_gl_enum_name(attach),
			piglit_get_gl_enum_name(actual_type));

		/* Return now and don't query the attachment name, because
		 * that would generate a GL error.
		 */
		return false;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      attach,
				              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				              &actual_name);

	if (actual_name != expect_name) {
		fprintf(stderr,
			"error: expected %d for %s attachment name, but found %d\n",
			expect_name, piglit_get_gl_enum_name(attach), actual_name);
		return false;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      attach,
				              GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE,
				              &actual_cube_map_face);

	if (actual_cube_map_face != expect_cube_map_face) {
		fprintf(stderr,
			"error: expected %s for %s attachment cube map face, but found %s\n",
			piglit_get_gl_enum_name(expect_cube_map_face),
			piglit_get_gl_enum_name(attach),
			piglit_get_gl_enum_name(actual_cube_map_face));
		return false;
	}

	return true;
}

enum piglit_result
piglit_display()
{
   return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	GLenum cube_map_face = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	GLuint fb;
	GLuint tex;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_depth_texture");

	glGenTextures(1, &tex);
	glGenFramebuffers(1, &fb);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glTexImage2D(cube_map_face,
		     0, /*level*/
		     GL_DEPTH_STENCIL,
		     200, 200, /*width, height*/
		     0, /*border*/
		     GL_DEPTH_STENCIL,
		     GL_UNSIGNED_INT_24_8,
		     NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			       GL_DEPTH_STENCIL_ATTACHMENT,
			       cube_map_face,
			       tex,
			       0); /*level*/

	pass = piglit_check_gl_error(0) && pass;

	pass = check_attachment(GL_DEPTH_ATTACHMENT, tex, cube_map_face) && pass;
	pass = check_attachment(GL_STENCIL_ATTACHMENT, tex, cube_map_face) && pass;
	pass = check_attachment(GL_DEPTH_STENCIL_ATTACHMENT, tex, cube_map_face) && pass;

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
