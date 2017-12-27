/*
 * Copyright © 2016 Intel Corporation
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
 * \file blit-corrupts-state.c
 * Tests for a bug in glBlitFramebuffer corrupting GL_DEPTH_STENCIL_TEXTURE_MODE
 *
 * The default state for GL_DEPTH_STENCIL_TEXTURE_MODE is GL_DEPTH_COMPONENT.
 * Create two GL_DEPTH_STENCIL textures and two framebuffer objects.  Attach
 * one texture to each of the FBOs, and blit stencil from one to the other.
 * After the blit operation verify that the state of
 * GL_DEPTH_STENCIL_TEXTURE_MODE has not changed.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

static bool
check_texture_state(GLenum target, unsigned line)
{
	GLint value;

	glGetTexParameteriv(target,
			    GL_DEPTH_STENCIL_TEXTURE_MODE,
			    &value);
	if (value != GL_DEPTH_COMPONENT) {
		printf("%s, %d: Expected GL_DEPTH_COMPONENT, got %s "
		       "(0x%04x).\n",
		       __func__, line,
		       piglit_get_gl_enum_name(value),
		       value);
		return false;
	}

	return true;
}

static void
setup_texture(GLenum target)
{
	/* All of the non-multisample targets should have the minification and
	 * the magnification set to GL_NEAREST.  Setting the filters for
	 * multisample targets results in a GL error.
	 */
	if (target != GL_TEXTURE_2D_MULTISAMPLE &&
	    target != GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	switch (target) {
	case GL_TEXTURE_1D:
		glTexImage1D(target,
			     0 /* level */,
			     GL_DEPTH24_STENCIL8,
			     16 /* width */,
			     0 /* border */,
			     GL_DEPTH_STENCIL,
			     GL_UNSIGNED_INT_24_8,
			     NULL);
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_1D_ARRAY:
		glTexImage2D(target,
			     0 /* level */,
			     GL_DEPTH24_STENCIL8,
			     16 /* width */,
			     16 /* height */,
			     0 /* border */,
			     GL_DEPTH_STENCIL,
			     GL_UNSIGNED_INT_24_8,
			     NULL);
		break;

	case GL_TEXTURE_CUBE_MAP:
		for (unsigned i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				     0 /* level */,
				     GL_DEPTH24_STENCIL8,
				     16 /* width */,
				     16 /* height */,
				     0 /* border */,
				     GL_DEPTH_STENCIL,
				     GL_UNSIGNED_INT_24_8,
				     NULL);
		}
		break;

	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexImage3D(target,
			     0 /* level */,
			     GL_DEPTH24_STENCIL8,
			     16 /* width */,
			     16 /* height */,
			     12 /* depth */,
			     0 /* border */,
			     GL_DEPTH_STENCIL,
			     GL_UNSIGNED_INT_24_8,
			     NULL);
		break;

	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexImage2DMultisample(target,
					2 /* samples */,
					GL_DEPTH24_STENCIL8,
					16 /* width */,
					16 /* height */,
					GL_TRUE /* fixedsamplelocations */);
		break;

	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(target,
					2 /* samples */,
					GL_DEPTH24_STENCIL8,
					16 /* width */,
					16 /* height */,
					2 /* depth */,
					GL_TRUE /* fixedsamplelocations */);
		break;
	}
}

static void
setup_fbo(GLenum target, GLenum textarget, GLuint attachment)
{
	GLenum status;

	switch (textarget) {
	case GL_TEXTURE_1D:
		glFramebufferTexture1D(target, GL_DEPTH_STENCIL_ATTACHMENT,
				       textarget, attachment,
				       0 /* level */);
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_2D_MULTISAMPLE:
	case GL_TEXTURE_RECTANGLE:
		glFramebufferTexture2D(target, GL_DEPTH_STENCIL_ATTACHMENT,
				       textarget, attachment,
				       0 /* level */);
		break;

	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glFramebufferTextureLayer(target, GL_DEPTH_STENCIL_ATTACHMENT,
					  attachment,
					  0 /* level */,
					  0 /* layer */);
		break;
	}

	status = glCheckFramebufferStatus(target);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %s (0x%04x).\n",
		       piglit_get_gl_enum_name(status),
		       status);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static const struct {
	GLenum target;
	const char *required_extension;
} test_vectors[] = {
	{ GL_TEXTURE_1D, NULL },
	{ GL_TEXTURE_2D, NULL },

	{ GL_TEXTURE_RECTANGLE, "GL_ARB_texture_rectangle" },
	{ GL_TEXTURE_2D_MULTISAMPLE, "GL_ARB_texture_multisample" },
	{ GL_TEXTURE_2D_MULTISAMPLE_ARRAY, "GL_ARB_texture_multisample" },

	/**
	 * These do not require any extensions because they are part of OpenGL
	 * 3.0.  This is especially important for GL_TEXTURE_CUBE_MAP.  This
	 * target existed before 3.0, but it could not be used for
	 * GL_DEPTH_COMPONENT or GL_DEPTH_STENCIL formats before then.
	 */
	/*@{*/
	{ GL_TEXTURE_1D_ARRAY, NULL },
	{ GL_TEXTURE_2D_ARRAY, NULL },
	{ GL_TEXTURE_CUBE_MAP, NULL },
	/*@}*/

	{ GL_TEXTURE_CUBE_MAP_ARRAY, "GL_ARB_texture_cube_map_array" },
};

static NORETURN void
usage_and_exit(const char *name)
{
	printf("Usage: %s <target>\n\n"
	       "Where <target> is one of:\n",
	       name);

	for (unsigned i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		const char *target_name =
			piglit_get_gl_enum_name(test_vectors[i].target);
		if (test_vectors[i].required_extension == NULL)
			printf("\t%s\n",
			       target_name);
		else
			printf("\t%s (requires %s)\n",
			       target_name,
			       test_vectors[i].required_extension);
	}

	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex[2];
	GLuint fbo[2];
	bool pass = true;
	GLenum target = 0;

	piglit_require_extension("GL_ARB_stencil_texturing");

	if (argc != 2)
		usage_and_exit(argv[0]);

	for (unsigned i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		if (strcmp(piglit_get_gl_enum_name(test_vectors[i].target),
			   argv[1]) == 0) {
			if (test_vectors[i].required_extension != NULL)
				piglit_require_extension(test_vectors[i].required_extension);

			target = test_vectors[i].target;
			break;
		}
	}

	if (target == 0)
		usage_and_exit(argv[0]);

	glGenTextures(ARRAY_SIZE(tex), tex);
	glGenFramebuffers(ARRAY_SIZE(fbo), fbo);

	glBindTexture(target, tex[0]);
	setup_texture(target);
	pass = check_texture_state(target, __LINE__) && pass;

	glBindTexture(target, tex[1]);
	setup_texture(target);
	pass = check_texture_state(target, __LINE__) && pass;

	glBindTexture(target, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);

	setup_fbo(GL_DRAW_FRAMEBUFFER, target, tex[0]);
	setup_fbo(GL_READ_FRAMEBUFFER, target, tex[1]);

	glBlitFramebuffer(0, 0, 15, 15,
			  0, 0, 15, 15,
			  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	glBindTexture(target, tex[0]);
	pass = check_texture_state(target, __LINE__) && pass;

	glBindTexture(target, tex[1]);
	pass = check_texture_state(target, __LINE__) && pass;

	glBindTexture(target, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glDeleteTextures(ARRAY_SIZE(tex), tex);
	glDeleteFramebuffers(ARRAY_SIZE(fbo), fbo);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
