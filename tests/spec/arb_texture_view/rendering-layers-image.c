/* Copyright © 2018 Danylo Piliaiev
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
 * Tests GL_ARB_texture_view interaction with ARB_shader_image_load_store.
 * Creates texture maps with different solid colors for each layer,
 * reads the framebuffer to ensure the rendered color is correct
 * and verifies that image has expected layers count.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

struct test_info
{
	GLenum target;
	const char* uniform_type;
	const char* img_layers_dimension;
	const char* img_access;
	int program;
};

struct test_info tests[] = {
	{GL_TEXTURE_1D_ARRAY, "image1DArray", "y", "ivec2(0, tex_layer)", -1},
	{GL_TEXTURE_2D_ARRAY, "image2DArray", "z", "ivec3(0, 0, tex_layer)", -1},
	{GL_TEXTURE_CUBE_MAP_ARRAY, "imageCubeArray", "z * 6", "ivec3(0, 0, tex_layer)", -1},
};

static bool
test_render_layers(const struct test_info *test)
{
	GLuint tex;
	const GLint width = 16, height = 16, layers = 12;
	const GLint num_layers[] = {7, 11, 2, 4};
	bool pass = true;

	glUseProgram(test->program);

	const GLint expected_layers_uniform = glGetUniformLocation(test->program, "expected_layers");

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(test->target, tex);

	if (test->target == GL_TEXTURE_1D_ARRAY) {
		glTexStorage2D(test->target, 1, GL_RGBA8, width, layers);
	} else {
		glTexStorage3D(test->target, 1, GL_RGBA8, width, height, layers);
	}

	/* Load each array layer with a different color texture */
	for (GLint l = 0; l < layers; l++) {
		GLubyte *buf = create_solid_image(width, height, 1, 4, l);

		if (buf != NULL) {
			if (test->target == GL_TEXTURE_1D_ARRAY) {
				glTexSubImage2D(test->target, 0, 0, l,
						width, 1, GL_RGBA,
						GL_UNSIGNED_BYTE, buf);
			} else {
				glTexSubImage3D(test->target, 0, 0, 0, l,
						width, height, 1, GL_RGBA,
						GL_UNSIGNED_BYTE, buf);
			}
			free(buf);
		}

	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Create a view of texture with restricted layers, bind it as image and draw a quad
	 * using a single layer in the view range which varies every loop, check image's layers
	 * count in a shader.
	 */
	for (GLint first_layer = 0; first_layer < ARRAY_SIZE(num_layers); first_layer++) {
		const int total_layers = test->target == GL_TEXTURE_CUBE_MAP_ARRAY ? 6 : num_layers[first_layer];

		GLuint view_tex;
		glGenTextures(1, &view_tex);
		glTextureView(view_tex, test->target, tex,  GL_RGBA8,
					0, 1, first_layer, total_layers);

		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, view_tex, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);

		glUniform1i(expected_layers_uniform, total_layers);

		glClear(GL_COLOR_BUFFER_BIT);

		draw_3d_depth(-1.0, -1.0, 2.0, 2.0, total_layers - 1);

		const int expected_layer = first_layer + total_layers - 1;

		GLfloat expected[4];
		expected[0] = Colors[expected_layer][0] / 255.0;
		expected[1] = Colors[expected_layer][1] / 255.0;
		expected[2] = Colors[expected_layer][2] / 255.0;
		expected[3] = 1.0;

		const int p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);

		piglit_present_results();

		if (!p) {
			printf("Wrong color for view min layer %d, expected layer %d\n",
				   first_layer, expected_layer);
			pass = false;
		}
		glDeleteTextures(1, &view_tex);
	}

	glDeleteTextures(1, &tex);
	return pass;
}

#define X(f, desc) \
	do { \
		const bool subtest_pass = (f); \
		piglit_report_subtest_result(subtest_pass \
						 ? PIGLIT_PASS : PIGLIT_FAIL, \
						 (desc)); \
		pass = pass && subtest_pass; \
	} while (0)

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	for (int test_idx = 0; test_idx < ARRAY_SIZE(tests); test_idx++) {
		const struct test_info *test = &tests[test_idx];
		char test_name[128];
		snprintf(test_name, sizeof(test_name), "layers rendering of %s", test->uniform_type);
		X(test_render_layers(test), test_name);
	}
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const char *vs =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"flat out int tex_layer;\n"
	"void main() { \n"
	"	gl_Position = vec4(piglit_vertex.xy, 0.0, 1.0);\n"
	"	tex_layer = int(piglit_vertex.z);\n"
	"}\n";

static const char *fs_template =
	"#version 150\n"
	"#extension GL_ARB_shader_image_size : enable\n"
	"#extension GL_ARB_shading_language_420pack : enable\n"
	"#extension GL_ARB_shader_image_load_store : enable\n"
	"flat in int tex_layer;\n"
	"layout(binding = 0, rgba8) uniform %s img;\n"
	"uniform int expected_layers;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"   if (imageSize(img).%s == expected_layers)\n"
	"		color = vec4(imageLoad(img, %s).rgb, 1.0);\n"
	"	else\n"
	"		color = vec4(0.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_shader_image_load_store");
	piglit_require_extension("GL_ARB_shader_image_size");

	for (int test_idx = 0; test_idx < ARRAY_SIZE(tests); test_idx++) {
		struct test_info *test = &tests[test_idx];

		char fs[512];
		snprintf(fs, sizeof(fs), fs_template, test->uniform_type,
			 test->img_layers_dimension, test->img_access);
		test->program = piglit_build_simple_program(vs, fs);
	}
}
