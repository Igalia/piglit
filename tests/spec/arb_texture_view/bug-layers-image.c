/* Copyright © 2019 Intel Corporation
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
 * Checks for a trivial bug in the mesa core code forgetting to clear
 * the texture unit _Layer variable.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"void main() { \n"
	"	gl_Position = vec4(piglit_vertex.xy, 0.0, 1.0);\n"
	"}\n";

static const char *fs =
	"#version 150\n"
	"#extension GL_ARB_shader_image_size : enable\n"
	"#extension GL_ARB_shading_language_420pack : enable\n"
	"#extension GL_ARB_shader_image_load_store : enable\n"
	"layout(binding = 0, rgba8) uniform image2D img;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"       color = vec4(imageLoad(img, ivec2(0, 0)).rgb, 1.0);\n"
	"}\n";

static int program;

static bool
test_render_layers(void)
{
	GLuint tex_array;
	const GLint width = 16, height = 16, layers = 12;
	bool pass = true;

	glUseProgram(program);

	glGenTextures(1, &tex_array);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layers);

	/* Load each array layer with a different color texture */
	for (GLint l = 0; l < layers; l++) {
		GLubyte *buf = create_solid_image(width, height, 1, 4, l);

		if (buf != NULL) {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l,
					width, height, 1, GL_RGBA,
					GL_UNSIGNED_BYTE, buf);
			free(buf);
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;


	/* Create a view of texture on the last layer as a 2D array
	 * texture.
	 */
	GLuint view_tex0;
	glGenTextures(1, &view_tex0);
	glTextureView(view_tex0, GL_TEXTURE_2D_ARRAY, tex_array,  GL_RGBA8,
		      0, 1, layers - 1, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, view_tex0, 0, GL_FALSE, layers - 1, GL_READ_ONLY, GL_RGBA8);

	glClear(GL_COLOR_BUFFER_BIT);

	draw_3d_depth(-1.0, -1.0, 2.0, 2.0, 0);

	/* Create another view of texture again on the last layer, but
	 * this time as a 2D texture. This will trigger a bug on i965
	 * because some internal variable doesn't get cleared up and
	 * it ends up adding the offset layer of the previous texture
	 * stored on the texture unit.
	 */
	GLuint view_tex1;
	glGenTextures(1, &view_tex1);
	glTextureView(view_tex1, GL_TEXTURE_2D, tex_array,  GL_RGBA8,
		      0, 1, layers - 1, 1);
	glBindImageTexture(0, view_tex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

	glClear(GL_COLOR_BUFFER_BIT);

	draw_3d_depth(-1.0, -1.0, 2.0, 2.0, 0);

	glDeleteTextures(1, &view_tex0);
	glDeleteTextures(1, &view_tex1);
	glDeleteTextures(1, &tex_array);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	return test_render_layers() ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_shader_image_load_store");
	piglit_require_extension("GL_ARB_shader_image_size");

	program = piglit_build_simple_program(vs, fs);
}
