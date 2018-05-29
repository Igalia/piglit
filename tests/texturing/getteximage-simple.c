/**
 * @file getteximage-simple.c
 *
 * Extremely basic test to check whether image data can be retrieved.
 *
 * Note that the texture is used in a full frame of rendering before
 * the readback, to ensure that buffer manager related code for uploading
 * texture images is executed before the readback.
 *
 * This used to crash for R300+bufmgr.
 *
 * This also used to stress test the blit methods in i965. The BLT engine only
 * supports pitch sizes up to but not including 32768 dwords. BLORP supports
 * even larger sizes.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_TYPE_VAL 1.0
#define PIX_TYPE GLfloat
#define TEX_TYPE GL_FLOAT
#define TEX_INT_FMT GL_RGBA32F
#define TEX_FMT GL_RGBA
#define CHANNELS_PER_PIXEL 4

static bool test_getteximage(PIX_TYPE *data, size_t data_size, GLint w, GLint h)
{
	PIX_TYPE *compare = (PIX_TYPE *)malloc(data_size);

	glGetTexImage(GL_TEXTURE_2D, 0, TEX_FMT, TEX_TYPE, compare);

	bool match = true;
	const unsigned data_channels = w * h / CHANNELS_PER_PIXEL;
	for (unsigned i = 0; i < data_channels; ++i) {
		if (data[i] != compare[i]) {
			const unsigned pixel = i / CHANNELS_PER_PIXEL;
			const unsigned pixel_channel = i % CHANNELS_PER_PIXEL;
			printf("GetTexImage() returns incorrect data in element %i\n", i);
			printf("    corresponding to (%i,%i) channel %i\n", pixel % w, pixel / w, pixel_channel);
			printf("    expected: %f\n", data[i]);
			printf("    got: %f\n", compare[i]);
			match = false;
			break;
		}
	}

	free(compare);
	return match;
}


enum piglit_result
piglit_display(void)
{
	GLsizei height = 2;
	GLsizei width;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &width);

	/* Upload random data to a texture with the given dimensions */
	const unsigned data_channels = width * height * CHANNELS_PER_PIXEL;
	const size_t data_size = data_channels * sizeof(PIX_TYPE);
	PIX_TYPE *data = (PIX_TYPE *)malloc(data_size);
	for (unsigned i = 0; i < data_channels; ++i)
		data[i] = ((float)rand() / RAND_MAX) * MAX_TYPE_VAL;
	glTexImage2D(GL_TEXTURE_2D, 0, TEX_INT_FMT, width, height, 0, TEX_FMT,
		     TEX_TYPE, data);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(1, 0);
	glVertex2f(1, 0);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(0, 1);
	glEnd();

	piglit_present_results();

	bool pass = test_getteximage(data, data_size, width, height);

	free(data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	if (TEX_TYPE == GL_FLOAT)
		piglit_require_extension("GL_ARB_texture_float");

	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	piglit_gen_ortho_projection(0.0, 1.0, 0.0, 1.0, -2.0, 6.0, GL_FALSE);
}
