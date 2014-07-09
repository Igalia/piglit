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
 * Tests GL_ARB_texture_view  rendering with various layers.
 * Creates texture maps with different  solid colors for each layer,
 * reads the framebuffer to ensure the rendered color is correct.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-rendering-levels";
static int tex_loc_2Darray;
static int prog2Darray;

/**
 * Views  with varying minimum  and number of layers 2D_ARRAY only
 */
static bool
test_render_layers(void)
{
	GLuint tex, new_tex;
	GLint width = 16, height = 16, layers = 8;
	GLint l;
	GLint numLayers[] = {7, 1, 2, 2};
	int expectedLayer;
	GLfloat expected[3];
	int p;
	bool pass = true;

	glUseProgram(prog2Darray);
	glUniform1i(tex_loc_2Darray, 0);

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layers);

	/* load each array layer with a different color texture */
	for (l = 0; l < layers; l++) {
		GLubyte *buf = create_solid_image(width, height, 1, 4, l);

		if (buf != NULL) {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l,
					width, height, 1, GL_RGBA,
					GL_UNSIGNED_BYTE, buf);
			free(buf);
		}

	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* create view of texture with restricted layers and draw quad */
	/* using a single layer in the view range which varies every loop */
	for (l = 0; l < ARRAY_SIZE(numLayers); l++) {
		glGenTextures(1, &new_tex);
		glTextureView(new_tex, GL_TEXTURE_2D_ARRAY, tex,  GL_RGBA8,
			      0, 1, l, numLayers[l]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, new_tex);

		glClear(GL_COLOR_BUFFER_BIT);

		expectedLayer = l + numLayers[l] - 1;
		draw_3d_depth(-1.0, -1.0, 2.0, 2.0, expectedLayer);

		expected[0] = Colors[expectedLayer][0] / 255.0;
		expected[1] = Colors[expectedLayer][1] / 255.0;
		expected[2] = Colors[expectedLayer][2] / 255.0;

		p = piglit_probe_pixel_rgb(piglit_width/2, piglit_height/2,
					   expected);

		piglit_present_results();

#if 0
		printf("for view min layer=%d expectedLayer=%d expected color=%f %f %f\n",
		       l, expectedLayer, expected[0], expected[1], expected[2]);
		sleep(1);
#endif

		if (!p) {
			printf("%s: wrong color for view min layer %d, expectedLayer %d\n",
			       TestName, l, expectedLayer);
			pass = false;
		}
		glDeleteTextures(1, &new_tex);
	}

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
	X(test_render_layers(), "2D layers rendering");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	char *vsCode;
	char *fsCode;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_EXT_texture_array");

	/* setup shaders and program object for 2DArray rendering */
	asprintf(&vsCode,
		 "void main()\n"
		 "{\n"
		 "    gl_Position = gl_Vertex;\n"
		 "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		 "}\n");
	asprintf(&fsCode,
		 "#extension GL_EXT_texture_array : enable\n"
		 "uniform sampler2DArray tex;\n"
		 "void main()\n"
		 "{\n"
		 "   vec4 color  = texture2DArray(tex, gl_TexCoord[0].xyz);\n"
		 "   gl_FragColor = vec4(color.xyz, 1.0);\n"
		 "}\n");
	prog2Darray = piglit_build_simple_program(vsCode, fsCode);
	free(fsCode);
	free(vsCode);
	tex_loc_2Darray = glGetUniformLocation(prog2Darray, "tex");

}
