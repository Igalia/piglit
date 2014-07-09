/*
 * Copyright © 2013 The Piglit project
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file ignore-adjacent-vertices.c
 *
 * Test that adjacent vertices are ignored when no geometry shader is active.
 * Draw the adjacency primitive in red and blend the non adjacency version in
 * green on top of it. Then test that the entire framebuffer is either yellow
 * or black.
 *
 * From the ARB_geometry_shader4 spec section 2.6.1:
 * "If a geometry shader is not active, the "adjacent" vertices are ignored."
 */

#include "piglit-util-gl.h"

struct primitive {
	GLenum type;
	int count;
	unsigned short indices[12];
};

static const struct primitives {
	struct primitive  adjacency, base;
} tests[] = {
	{
		{GL_LINES_ADJACENCY, 8,
		{4, 5, 6, 7, 8, 9, 10, 11} },
		{GL_LINES, 4,
		{5, 6, 9, 10} }
	},
	{
		{GL_LINE_STRIP_ADJACENCY, 6,
		{4, 5, 6, 10, 9, 8} },
		{GL_LINE_STRIP, 4,
		{5, 6, 10, 9} }
	},
	{
		{GL_TRIANGLES_ADJACENCY, 12,
		{9, 4, 5, 6, 10, 14, 6, 11, 10, 9, 5, 1} },
		{GL_TRIANGLES, 6,
		{9, 5, 10, 6, 10, 5} }
	},
	{
		{GL_TRIANGLE_STRIP_ADJACENCY, 8,
		{9, 4, 5, 14, 10, 1, 6, 11} },
		{GL_TRIANGLE_STRIP, 4,
		{9, 5, 10, 6} }
	}
};

static const float vertex_data[] = {
	-1, 1, -1/3., 1, 1/3., 1, 1, 1,
	0, 1/3., -1/3., 1/3., 1/3., 1/3., 1, 1/3.,
	0, -1/3., -1/3., -1/3., 1/3., -1/3., 1, -1/3.,
	0, 0, -1/3., 0, 1/3., 0, 1, 0,
};

static const char vs_text[] =
	"attribute vec4 vertex;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"}\n";

static const char fs_text[] =
	"uniform vec4 color;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";

static const struct primitives *test;
static bool indexed = false;
static bool use_core = false;

static void
parse_cmd_line(int argc, char **argv);

PIGLIT_GL_TEST_CONFIG_BEGIN
	parse_cmd_line(argc, argv);
	if (!use_core) {
		config.supports_gl_compat_version = 20;
		config.supports_gl_core_version = 31;
	} else {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	}
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

GLuint color_uniform;

/* Check that the framebuffer is yellow and black. */
static bool
check_framebuffer(void)
{
	int y, x;
	uint32_t *buffer = malloc(sizeof(uint32_t) * piglit_width *
				  piglit_height);
#ifdef __BIG_ENDIAN__
	const GLenum type = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
	const GLenum type = GL_UNSIGNED_INT_8_8_8_8;
#endif

	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, type, buffer);

	for (y = 0; y < piglit_height; ++y) {
		for (x = 0; x < piglit_width; ++x) {
			uint32_t val = buffer[y * piglit_width + x] &
				0xFFFFFF00;

			if (val != 0 && val != 0xFFFF0000) {
				fprintf(stderr,
					"FAIL: Rendered primitives differ.\n");
				return false;
			}
		}
	}

	free(buffer);
	return true;
}

/* Parse command line arguments.
 *
 * Recognized command line arguments are:
 *     * The primitive type with adjacency to test (one of GL_LINES_ADJACENCY,
 *       GL_LINE_STRIP_ADJACENCY, GL_TRIANGLES_ADJACENCY or
 *       GL_TRIANGLE_STRIP_ADJACENCY).
 *     * The optional argument "indexed" to use indexed drawing.
 *     * The optional argument "core" to use GLSL 1.50
 */
static void
parse_cmd_line(int argc, char **argv)
{
	int i, j;

	for (i = 1; i < argc; i++) {
		for (j = 0; j < ARRAY_SIZE(tests); j++) {
			if (strcmp(argv[i],
			    piglit_get_prim_name(tests[j].adjacency.type)) == 0)
				test = &tests[j];
		}
		if (strcmp(argv[i], "indexed") == 0)
			indexed = true;
		else if (strcmp(argv[i], "core") == 0)
			use_core = true;
	}

	if (test == NULL) {
		fprintf(stderr, "Please specify the adjacent primitive type "
			"to test on the command line\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
		GLuint array_bufs[2];
	GLuint array;
	GLuint prog;

	if (!use_core)
		piglit_require_extension("GL_ARB_geometry_shader4");

	/* Bind Vertex Data */
	glGenVertexArrays(1, &array);
	glBindVertexArray(array);
	glGenBuffers(2, array_bufs);
	glBindBuffer(GL_ARRAY_BUFFER, array_bufs[0]);
	if (indexed)
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data),
			     vertex_data, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, array_bufs[1]);

	/* Create shader. */
	prog = piglit_build_simple_program(vs_text, fs_text);
	glBindAttribLocation(prog, 0, "vertex");
	glLinkProgram(prog);
	color_uniform = glGetUniformLocation(prog, "color");
	glUseProgram(prog);

	/* Enable blending. */
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
}

static void
draw(const struct primitive prim)
{
	if (indexed) {
		/* Upload index data and draw. */
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, prim.count * 2,
			     prim.indices, GL_STREAM_DRAW);
		glDrawElements(prim.type, prim.count, GL_UNSIGNED_SHORT, NULL);
	} else {
		int i;
		float data[24];

		/* Build vertex data, upload it and draw. */
		for (i = 0; i < prim.count; ++i) {
			data[2 * i + 0] = vertex_data[2 * prim.indices[i] + 0];
			data[2 * i + 1] = vertex_data[2 * prim.indices[i] + 1];
		}
		glBufferData(GL_ARRAY_BUFFER, prim.count * 2 * 4, data,
			     GL_STREAM_DRAW);
		glDrawArrays(prim.type, 0, prim.count);
	}
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	const float red[] = {1, 0, 0, 1};
	const float green[] = {0, 1, 0, 1};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw adjacency primitive red. */
	glUniform4fv(color_uniform, 1, red);
	draw(test->adjacency);

	/* Draw normal primitive green. */
	glUniform4fv(color_uniform, 1, green);
	draw(test->base);

	pass = check_framebuffer() && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (!piglit_automatic)
		piglit_present_results();

	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
