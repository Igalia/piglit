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
 * Tests GL_ARB_texture_view rendering with various texture targets.
 * Creates texture maps with different  solid colors for each level or layer
 * reads the framebuffer to ensure the rendered color is correct.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-rendering-target";
static int prog3D, prog2Darray, prog2D, prog1D;

/**
 * Simple views  of textures; test rendering with various texture view targets
 */
static bool
test_render_with_targets(GLenum target)
{
	GLuint tex, newTex;
	GLint width = 128, height = 64, depth = 4, levels = 8;
	GLint l;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	switch (target) {
	case GL_TEXTURE_1D:
		glTexStorage1D(target, levels, GL_RGBA8, width);
		height = 1;
		depth = 1;
		break;
	case GL_TEXTURE_2D:
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		depth = 1;
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(target, levels, GL_RGBA8, width, height, depth);
		break;
	default:
		/* only handle subset of legal targets */
		piglit_report_result(PIGLIT_FAIL);
		assert(!"Illegal target for test_render_with_targets()\n");
		break;
	}

	/* load each mipmap with a different color texture */
	for (l = 0; l < levels; l++) {
		GLubyte *buf = create_solid_image(width, height, depth, 4, l);

		if (buf != NULL) {
			switch(target) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(GL_TEXTURE_1D, l, 0, width,
						GL_RGBA, GL_UNSIGNED_BYTE, buf);
				break;
			case GL_TEXTURE_2D:
				glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0, width,
						height, GL_RGBA,
						GL_UNSIGNED_BYTE, buf);
				break;
			case GL_TEXTURE_3D:
			case GL_TEXTURE_2D_ARRAY:
				glTexSubImage3D(target, l, 0, 0, 0, width,
						height, depth, GL_RGBA,
						GL_UNSIGNED_BYTE, buf);
				break;
			}
			free(buf);
		} else {
			piglit_report_result(PIGLIT_FAIL);
			assert(!"create_solid_image() failed\n");
		}

		if (width > 1)
			width /= 2;
		if (height > 1)
			height /= 2;
		if (depth > 1 && target == GL_TEXTURE_3D)
			depth /= 2;
	}

	if (piglit_check_gl_error(GL_NO_ERROR) == GL_FALSE) {
		printf("%s: Found gl errors prior to testing glTextureView\n",
				   TestName);
		glDeleteTextures(1, &tex);

		return false;
	}

	/* create view of texture and bind it to target */
	glGenTextures(1, &newTex);
	glTextureView(newTex, target, tex,  GL_RGBA8, 0, levels, 0, 1);
	glDeleteTextures(1, &tex);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, newTex);

	/* draw a quad/line using each texture mipmap level */
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	for (l = 0; l < levels; l++) {
		GLfloat expected[4];
		int p;

		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, l);
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, l);

		glClear(GL_COLOR_BUFFER_BIT);

		switch (target) {
		case GL_TEXTURE_1D:
			glUseProgram(prog1D);
			piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0, 0.0, 0.0,
					     1.0, 1.0);
			break;
		case GL_TEXTURE_2D:
			glUseProgram(prog2D);
			piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0, 0.0, 0.0,
					     1.0, 1.0);
			break;
		case GL_TEXTURE_2D_ARRAY:
			glUseProgram(prog2Darray);
			draw_3d_depth(-1.0, -1.0, 2.0, 2.0, l);
			break;
		case GL_TEXTURE_3D:
			glUseProgram(prog3D);
			draw_3d_depth(-1.0, -1.0, 2.0, 2.0, l);
			break;
		}

		expected[0] = Colors[l][0] / 255.0;
		expected[1] = Colors[l][1] / 255.0;
		expected[2] = Colors[l][2] / 255.0;
		expected[3] = 1.0;

		p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2,
					    expected);

		piglit_present_results();

#if 0		/* debug */
		printf("for level=%d, target=%s, expected color=%f %f %f\n",
			   l, piglit_get_gl_enum_name(target), expected[0],
				   expected[1], expected[2]);
		sleep(1);
#endif

		if (!p) {
			printf("%s: wrong color for mipmap level %d\n",
			       TestName, l);
			pass = false;
		}
	}
	glDeleteTextures(1, &newTex);

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
#ifdef PIGLIT_USE_OPENGL
	X(test_render_with_targets(GL_TEXTURE_1D), "1D view rendering");
#endif
	X(test_render_with_targets(GL_TEXTURE_2D), "2D view rendering");
	X(test_render_with_targets(GL_TEXTURE_3D), "3D view rendering");
	X(test_render_with_targets(GL_TEXTURE_2D_ARRAY),
	  "2D Array view rendering");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

#ifdef PIGLIT_USE_OPENGL
#define GLSL_VERSION "130"
#else
#define GLSL_VERSION "310 es"
#endif

static const char *vs =
	"#version " GLSL_VERSION "\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec3 texcoord;\n"
	"void main() { \n"
	"	gl_Position = vec4(piglit_vertex.xy, 0.0, 1.0);\n"
	"	texcoord = vec3(piglit_texcoord, piglit_vertex.z);\n"
	"}\n";

static const char *fs_3d =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler3D;\n"
	"#endif\n"
	"in vec3 texcoord;\n"
	"uniform sampler3D tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = vec4(texture(tex, texcoord).xyz, 1.0);\n"
	"}\n";

static const char *fs_2darray =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2DArray;\n"
	"#endif\n"
	"in vec3 texcoord;\n"
	"uniform sampler2DArray tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = vec4(texture(tex, texcoord).xyz, 1.0);\n"
	"}\n";

static const char *fs_2d =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"in vec3 texcoord;\n"
	"uniform sampler2D tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = vec4(texture(tex, texcoord.xy).xyz, 1.0);\n"
	"}\n";

#ifdef PIGLIT_USE_OPENGL
static const char *fs_1d =
	"#version " GLSL_VERSION "\n"
	"in vec3 texcoord;\n"
	"uniform sampler1D tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = vec4(texture(tex, texcoord.x).xyz, 1.0);\n"
	"}\n";
#endif

void
piglit_init(int argc, char **argv)
{
#ifdef PIGLIT_USE_OPENGL
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_EXT_texture_array");
#else
	piglit_require_extension("GL_OES_texture_view");
#endif

	prog3D = piglit_build_simple_program(vs, fs_3d);
	prog2Darray = piglit_build_simple_program(vs, fs_2darray);
	prog2D = piglit_build_simple_program(vs, fs_2d);

#ifdef PIGLIT_USE_OPENGL
	prog1D = piglit_build_simple_program(vs, fs_1d);
#endif
}
