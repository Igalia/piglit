/**
 * @file gettextureimage-simple.c
 *
 * Extremely basic test to check whether image data can be retrieved.
 *
 * Note that the texture is used in a full frame of rendering before
 * the readback, to ensure that buffer manager related code for uploading
 * texture images is executed before the readback.
 *
 * This used to crash for R300+bufmgr.
 *
 * Adapted for testing glGetTextureImage in ARB_direct_state_access by
 * Laura Ekstrand <laura@jlekstrand.net>, November 2014.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLubyte data[4096]; /* 64*16*4 */
static GLuint name; /* texture name */

static int test_getteximage(void)
{
	GLubyte compare[4096];
	int i;

	glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			  sizeof(compare), compare);

	for(i = 0; i < 4096; ++i) {
		if (data[i] != compare[i]) {
			printf("GetTextureImage() returns incorrect data in byte %i\n", i);
			printf("    corresponding to (%i,%i) channel %i\n", i / 64, (i / 4) % 16, i % 4);
			printf("    expected: %i\n", data[i]);
			printf("    got: %i\n", compare[i]);
			return 0;
		}
	}

	return 1;
}

enum piglit_result
piglit_display(void)
{
	int pass;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTextureUnit(0, name);
	piglit_draw_rect_tex(0, 0, 1, 1, 0, 0, 1, 1);

	piglit_present_results();

	pass = test_getteximage();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage");

	for(i = 0; i < 4096; ++i)
		data[i] = rand() & 0xff;

	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(name, 1, GL_RGBA8, 64, 16);
	glTextureSubImage2D(name, 0, 0, 0, 64, 16, GL_RGBA, GL_UNSIGNED_BYTE,
			    data);

	piglit_gen_ortho_projection(0.0, 1.0, 0.0, 1.0, -2.0, 6.0, GL_FALSE);
}
