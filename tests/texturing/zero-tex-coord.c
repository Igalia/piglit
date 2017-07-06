/*
 * Copyright © 2015 Intel Corporation
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
 * Authors:
 *    Neil Roberts <neil@linux.intel.com>
 *
 */

/** @file zero-tex-coord.c
 *
 * Tests various texture sampling functions using constant 0 values
 * for the arguments. The i965 driver has optimisations for trailing 0
 * arguments to sampler messages so the intention is to test these
 * code paths.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

struct sample_function {
	const char *name;
	const char *snippet;
	int glsl_version;
	const char *extension;
};

static const struct sample_function
sample_functions[] = {
	{ "texture2D", "texture2D(tex, vec2(0.0))", 0 },
	{ "bias", "texture2D(tex, vec2(0.0), 0.0)", 0 },
	{ "textureGrad",
	  "textureGrad(tex, vec2(0.0), vec2(0.0), vec2(0.0))",
	  130 },
	{ "texelFetch", "texelFetch(tex, ivec2(0), 0)", 130 },
	{ "textureLod",
	  "textureLod(tex, vec2(0.0), 0.0)",
	  130 },
	{ "textureSize",
	  "textureSize(tex, 0) == ivec2(4) ? "
	  "vec4(0.0, 1.0, 0.0, 1.0) : "
	  "vec4(1.0, 0.0, 0.0, 1.0)",
	  130 },
	{ "textureQueryLOD",
	  "textureQueryLOD(tex, vec2(0.0)).x == 0.0 ? "
	  "vec4(0.0, 1.0, 0.0, 1.0) : "
	  "vec4(1.0, 0.0, 0.0, 1.0)",
	  0, "GL_ARB_texture_query_lod" },
	{ "textureGather",
	  "textureGather(tex, vec2(0.0), 0) == vec4(0.0) ? "
	  "vec4(0.0, 1.0, 0.0, 1.0) : "
	  "vec4(1.0, 0.0, 0.0, 1.0)",
	  130, "GL_ARB_gpu_shader5" },
};

static const char
vertex_source[] =
	"attribute vec2 piglit_vertex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"}\n";

static const char
fragment_source[] =
	"uniform sampler2D tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_FragColor = SNIPPET;\n"
	"}\n";

const struct sample_function *
sample_function = sample_functions;

static const GLfloat
green[] = { 0.0f, 1.0f, 0.0f, 1.0f };

static GLuint
make_tex(void)
{
	/* Make a fully mipmapped texture with a green texel on at the
	 * 0,0 position on the largest mip image and a red texel for
	 * all of the other positions */

	GLuint tex;
	int size = 4;
	int level = 0;
	GLubyte *image, *p;
	int y, x;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);

	while (size >= 1) {
		p = image = malloc(size * size * 4);

		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				if (level == 0 && y == 0 && x == 0) {
					*(p++) = 0x00;
					*(p++) = 0xff;
				} else {
					*(p++) = 0xff;
					*(p++) = 0x00;
				}
				*(p++) = 0x00;
				*(p++) = 0x00;
			}
		}

		glTexImage2D(GL_TEXTURE_2D,
			     level,
			     GL_RGBA,
			     size, size,
			     0, /* border */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     image);

		free(image);

		size /= 2;
		level++;
	}

	return tex;
}

static GLuint
make_program(void)
{
	const char *snippet = sample_function->snippet;
	int glsl_version = sample_function->glsl_version;
	int snippet_length = strlen(snippet);
	int snippet_pos = strstr(fragment_source, "SNIPPET") - fragment_source;
	GLuint program;
	char source[1000], *p = source;

	if (glsl_version > 0) {
		p += sprintf(p, "#version %i\n", glsl_version);
		piglit_require_GLSL_version(glsl_version);
	}

	if (sample_function->extension) {
		p += sprintf(p,
			     "#extension %s : require\n",
			     sample_function->extension);
		piglit_require_extension(sample_function->extension);
	}

	memcpy(p, fragment_source, snippet_pos);
	p += snippet_pos;
	memcpy(p, snippet, snippet_length);
	p += snippet_length;
	memcpy(p,
	       fragment_source + snippet_pos + 7,
	       sizeof fragment_source - snippet_pos - 7);

	program = piglit_build_simple_program(vertex_source, source);

	return program;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint program, tex;
	GLint tex_location;

	tex = make_tex();
	program = make_program();

	glUseProgram(program);
	tex_location = glGetUniformLocation(program, "tex");
	glUniform1i(tex_location, 0);

	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);

	glUseProgram(0);

	glDeleteTextures(1, &tex);
	glDeleteProgram(program);

	pass = piglit_probe_rect_rgb(0, 0,
				     piglit_width, piglit_height,
				     green) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;

	if (argc > 1) {
		for (i = 0; i < ARRAY_SIZE(sample_functions); i++) {
			if (!strcmp(sample_functions[i].name, argv[1])) {
				sample_function = sample_functions + i;
				goto found;
			}
		}

		fprintf(stderr, "Unknown function: %s\n", argv[1]);
		piglit_report_result(PIGLIT_FAIL);

	found:
		(void) 0;
	}

	piglit_require_GLSL();
}
