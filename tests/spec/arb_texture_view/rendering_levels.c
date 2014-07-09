/*
 * Copyright © 2013 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 * Tests GL_ARB_texture_view  rendering with various levels.
 * Creates texture maps with different  solid colors for each level,
 * reads the framebuffer to ensure the rendered color is correct.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-rendering-levels";

/**
 * Texture views with varying minimum and number of levels, 2D only
 */
static bool
test_render_levels(void)
{
	GLuint tex, new_tex;
	GLint width = 4096, height = 4096, levels =13;
	GLuint numLevels[] = {3,2,2,1};
	GLint l;
	int expectedLevel;
	GLfloat expected[3];
	int p;
	bool pass = true;

	glUseProgram(0);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexStorage2D(GL_TEXTURE_2D, levels, GL_RGBA8, width, height);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* load each mipmap with a different color texture */
	for (l = 0; l < levels; l++) {
		GLubyte *buf = create_solid_image(width, height, 1, 4, l);

		if (buf != NULL) {
			glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0, width, height,
					GL_RGBA, GL_UNSIGNED_BYTE, buf);
			free(buf);
		}

		if (width > 1)
			width /= 2;
		if (height > 1)
			height /= 2;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* create view of texture with restricted levels and draw quad */
	/* using smallest mip level in the view range which varies every loop */
	for (l = 0; l < ARRAY_SIZE(numLevels); l++) {
		glGenTextures(1, &new_tex);
		glTextureView(new_tex, GL_TEXTURE_2D, tex,  GL_RGBA8, l,
			      numLevels[l], 0, 1);
		glBindTexture(GL_TEXTURE_2D, new_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels-1);

		glClear(GL_COLOR_BUFFER_BIT);

		piglit_draw_rect_tex(-1.0, -1.0, 2.0/(float) (l+2),
				     2.0/ (float) (l+2), 0.0, 0.0, 1.0, 1.0);

		expectedLevel = l + numLevels[l] - 1;
		expected[0] = Colors[expectedLevel][0] / 255.0;
		expected[1] = Colors[expectedLevel][1] / 255.0;
		expected[2] = Colors[expectedLevel][2] / 255.0;

		p = piglit_probe_pixel_rgb(piglit_width/(2*(l+3)),
					   piglit_height/(2*(l+3)), expected);

		piglit_present_results();

#if 0
		{  /* debug */
			GLint param;
			glGetTexParameteriv(GL_TEXTURE_2D,
					    GL_TEXTURE_BASE_LEVEL, &param);
			printf("for view min level=%d base_level=%d exp color=%f %f %f\n",
			       l, param, expected[0], expected[1], expected[2]);
			glGetTexParameteriv(GL_TEXTURE_2D,
					    GL_TEXTURE_MAX_LEVEL, &param);
			printf("max_level=%d\n", param);
			glGetTexParameteriv(GL_TEXTURE_2D,
					    GL_TEXTURE_VIEW_MIN_LEVEL, &param);
			printf("view min_level=%d\n", param);
			glGetTexParameteriv(GL_TEXTURE_2D,
					    GL_TEXTURE_VIEW_NUM_LEVELS, &param);
			printf("view num_level=%d\n", param);
			glGetTexParameteriv(GL_TEXTURE_2D,
					    GL_TEXTURE_IMMUTABLE_LEVELS,
					    &param);
			printf("immutable levels=%d\n", param);
			sleep(1);
		}
#endif

		if (!p) {
			printf("%s: wrong color for view min level %d, expectedLevel %d\n",
				   TestName, l, expectedLevel);
			pass = false;
		}
		glDeleteTextures(1, &new_tex);
	}

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	return pass;
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	X(test_render_levels(), "2D levels rendering");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
}
