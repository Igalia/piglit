/**
 * @file crash-texparameter-before-teximage.c
 *
 * Test case for "crash before first TexImage"; this happened in the R300
 * driver before Mesa commit c1fb448ce8dd98f8e5fd5a39707f96cc14535bd4
 *
 * This bug was originally triggered by Glest.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	glViewport(0, 0, piglit_width, piglit_height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	if (piglit_is_extension_supported("GL_ARB_shadow"))
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA);
	if (piglit_is_extension_supported("GL_ARB_shadow_ambient"))
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
}
