/*
 * Copyright © 2008, 2014 Intel Corporation
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

#include "piglit-util-gl.h"
#include "dsa-utils.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 20;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"attribute vec4 piglit_vertex;\n"
	"#else\n"
	"in vec4 piglit_vertex;\n"
	"#endif\n"
	"uniform mat3 xform;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_Position = vec4((xform * piglit_vertex.xyw).xy, 0, 1);\n"
	"}\n"
	;

static const char fs_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"#define piglit_color gl_FragColor\n"
	"#else\n"
	"out vec4 piglit_color;\n"
	"#endif\n"
	"uniform vec3 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        piglit_color = vec4(color, 1);\n"
	"}\n"
	;

static GLuint builder_prog = 0;
static GLuint texture2D_prog = 0;
static GLuint texture_rect_prog = 0;

static bool inrect(int x, int y, int x1, int y1, int x2, int y2)
{
	if (x >= x1 && x < x2 && y >= y1 && y < y2)
		return true;
	else
		return false;
}

static bool
check_results(int dstx, int dsty, int w, int h)
{
	GLfloat *results;
	bool pass = true;
	int x, y;

	results = malloc(w * h * 4 * sizeof(GLfloat));

	/* Check the results */
	glReadPixels(dstx, dsty, w, h, GL_RGBA, GL_FLOAT, results);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GLfloat expected[3];

			if (inrect(x, y, 5, h/2, w - 5, h - 5)) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
			} else if (inrect(x, y, 5, 5, w - 5, h/2)) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
			}

			if (results[(y * w + x) * 4 + 0] != expected[0] ||
			    results[(y * w + x) * 4 + 1] != expected[1] ||
			    results[(y * w + x) * 4 + 2] != expected[2]) {
				printf("Expected at (%d,%d): %f,%f,%f\n",
				       x, y,
				       expected[0], expected[1], expected[2]);
				printf("Probed at   (%d,%d): %f,%f,%f\n",
				       x, y,
				       results[(y * w + x) * 4 + 0],
				       results[(y * w + x) * 4 + 1],
				       results[(y * w + x) * 4 + 2]);
				pass = false;
			}
		}
	}

	free(results);
	return pass;
}

static bool
do_row(int srcy, int srcw, int srch, GLenum target)
{
	int srcx = 20;
	int dstx = 80, dsty = srcy;
	int dstx2 = 140, dsty2 = srcy;
	int remain_width;
	int remain_height;
	GLuint texname;
	bool pass = true;
	GLint color_loc;
	GLuint prog;

	/* Rectangle textures use coordinates on the range [0..w]x[0..h],
	 * where as all other textures use coordinates on the range
	 * [0..1]x[0..1].
	 */
	const GLfloat tex_s_max = (target == GL_TEXTURE_RECTANGLE_ARB)
		? (float)srcw : 1.0;
	const GLfloat tex_t_max = (target == GL_TEXTURE_RECTANGLE_ARB)
		? (float)srch : 1.0;


	/* Draw the object we're going to copy */
	glUseProgram(builder_prog);
	color_loc = glGetUniformLocation(builder_prog, "color");
	dsa_set_xform(builder_prog, piglit_width, piglit_height);

	glUniform3f(color_loc, 1.0, 0.0, 0.0);
	piglit_draw_rect(srcx, srcy, srcw, srch);
	glUniform3f(color_loc, 0.0, 1.0, 0.0);
	piglit_draw_rect(srcx + 5, srcy + 5, srcw - 10, srch/2 - 5);
	glUniform3f(color_loc, 0.0, 0.0, 1.0);
	piglit_draw_rect(srcx + 5, srcy + srch/2, srcw - 10, srch - 5 - srch/2);

	/* Create a texture image and copy it in */
	glGenTextures(1, &texname);
	glBindTexture(target, texname);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* The default mode is GL_REPEAT, and this mode is invalid for
	 * GL_TEXTURE_RECTANGLE_ARB textures.
	 */
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(target, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glCopyTextureSubImage2D(texname, 0,
				0, 0, /* offset in image */
				srcx, srcy, /* offset in readbuffer */
				srcw, srch);

	/* Draw the texture image out */
	switch (target) {
	case GL_TEXTURE_2D:
		prog = texture2D_prog;
		break;
	case GL_TEXTURE_RECTANGLE_ARB:
		prog = texture_rect_prog;
		break;
	default:
		fprintf(stderr, "Invalid texture target.\n");
		return false;
	}

	glUseProgram(prog);
	dsa_set_xform(prog, piglit_width, piglit_height);

	piglit_draw_rect_tex(dstx, dsty, srcw, srch,
			     0.0, 0.0, tex_s_max, tex_t_max);

	glTexImage2D(target, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	remain_width = srcw - (srcw / 2);
	remain_height = srch - (srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				0, 0, /* offset in image */
				srcx, srcy, /* offset in readbuffer */
				srcw / 2, srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				srcw / 2, 0, /* offset in image */
				srcx + srcw / 2, srcy, /* " in readbuffer */
				remain_width, srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				0, srch / 2, /* offset in image */
				srcx, srcy + srch / 2, /* " in readbuffer */
				srcw / 2, remain_height);
	glCopyTextureSubImage2D(texname, 0,
				srcw / 2, srch / 2, /* offset in image */
				srcx + srcw / 2, srcy + srch / 2, /*" in rb */
				remain_width, remain_height);

	/* Draw the texture image out */
	piglit_draw_rect_tex(dstx2, dsty2, srcw, srch,
			     0.0, 0.0, tex_s_max, tex_t_max);

	glDeleteTextures(1, &texname);

	printf("Checking %s, rect 1:\n", piglit_get_gl_enum_name(target));
	pass = check_results(dstx, dsty, srcw, srch) && pass;
	printf("Checking %s, rect 2:\n", piglit_get_gl_enum_name(target));
	pass = check_results(dstx2, dsty2, srcw, srch) && pass;

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass;
	int srcy = 5;


	glClear(GL_COLOR_BUFFER_BIT);


	/* Test plain old 2D textures.
	 */
	pass = do_row(srcy, 32, 32, GL_TEXTURE_2D);
	srcy += 33 + 5;


	/* Test non-power-of-two 2D textures.
	 */
	pass = do_row(srcy, 31, 13, GL_TEXTURE_2D) && pass;
	srcy += 15;
	pass = do_row(srcy, 11, 34, GL_TEXTURE_2D) && pass;
	srcy += 35 + 5;


	/* Test non-power-of-two 2D textures.
	 */
	if (texture_rect_prog != 0) {
		pass = do_row(srcy, 31, 13, GL_TEXTURE_RECTANGLE_ARB) && pass;
		srcy += 14;
		pass = do_row(srcy, 11, 34, GL_TEXTURE_RECTANGLE_ARB) && pass;
		srcy += 35 + 5;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	char *vs_source;
	char *fs_source;
	bool es;
	int major;
	int minor;
	const char * ver;


	piglit_require_extension("GL_ARB_direct_state_access");

	glClearColor(0.5, 0.5, 0.5, 1.0);

	piglit_get_glsl_version(&es, &major, &minor);
	ver = ((major * 100 + minor) >= 140) ? "140" : "110";

	(void)!asprintf(&vs_source, vs_template, ver);
	(void)!asprintf(&fs_source, fs_template, ver);

	builder_prog = piglit_build_simple_program(vs_source, fs_source);

	free(vs_source);
	free(fs_source);

	texture2D_prog = dsa_create_program(GL_TEXTURE_2D);

	if (piglit_is_extension_supported("GL_ARB_texture_rectangle"))
		texture_rect_prog = dsa_create_program(GL_TEXTURE_RECTANGLE_ARB);
}
