/*
 * Copyright © 2016 Broadcom Limited
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


/** @file drawarrays-vertex-count.c
 *
 * Tests glDrawArrays with large vertex counts and a start vertex
 * offset.  Catches a limitation of the vc4 hardware where
 * glDrawArrays() with a large count ends up truncating the high 16
 * bits of vertex indices.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

static GLenum primtype;
static unsigned int count;
static bool use_vbo;

static const float green[4] = {0.0, 1.0, 0.0, 0.0};
static const float black[4] = {0.0, 0.0, 0.0, 0.0};
static const float red[4] =   {1.0, 0.0, 0.0, 0.0};

static int v_from_end(int count, int prims, int primsize)
{
	int v = count - prims * primsize;
	return v - v % primsize;
}

/* Probes for a green filled rectangle in the screen, surrounded by black. */
static bool
probe_rect(int x, int y, int w, int h)
{
	return (piglit_probe_rect_rgba(x, y, w, h, green) &&
		piglit_probe_rect_rgba(0, 0, piglit_width, y, black) &&
		piglit_probe_rect_rgba(0, y, x, h, black) &&
		piglit_probe_rect_rgba(x + w, y,
				       piglit_width - (x + w), h, black) &&
		piglit_probe_rect_rgba(0, y + h,
				       piglit_width, piglit_height - (y + h),
				       black));
}

/* Probes for a green outlined rectangle in the screen, surrounded by black. */
static bool
probe_line_rect(int x1, int y1, int x2, int y2)
{
	int probe_w = x2 - x1 - 2;
	int probe_h = y2 - y1 - 2;

	/* Note that GL line rasterization may not include the endpoints.  */
	return (/* rect */
		piglit_probe_rect_rgba(x1 + 1, y1, probe_w, 1, green) &&
		piglit_probe_rect_rgba(x1 + 1, y2, probe_w, 1, green) &&
		piglit_probe_rect_rgba(x1, y1 + 1, 1, probe_h, green) &&
		piglit_probe_rect_rgba(x2, y1 + 1, 1, probe_h, green) &&
		/* inside the rect */
		piglit_probe_rect_rgba(x1 + 1, y1 + 1,
				       probe_w, probe_h, black) &&
		/* outside the rect */
		piglit_probe_rect_rgba(0, 0, piglit_width, y1, black) &&
		piglit_probe_rect_rgba(0, y1,
				       x1, y2 - y1, black) &&
		piglit_probe_rect_rgba(x2 + 1, y1,
				       piglit_width - x2, y2 - y1, black) &&
		piglit_probe_rect_rgba(0, y2 + 1,
				       piglit_width, piglit_height - (y2 + 1),
				       black));
}

/* Sets a range of the color array to a spefific color. */
static void
set_colors(float *colors, unsigned int start, unsigned int count,
	   const float *color)
{
	unsigned int i;

	for (i = start; i < start + count; i++)
		memcpy(&colors[i * 4], color, 4 * sizeof(float));
}

void set_point(float *verts, int p, float x, float y)
{
	verts[p * 2 + 0] = x;
	verts[p * 2 + 1] = y;
}

enum piglit_result
piglit_display(void)
{
	/* Start vertex offset for DrawArrays. */
	unsigned int sv = count / 3;
	/* Vertex index for the primitive we care about */
	unsigned int v;
	bool pass = true;
	char *data;
	float *vert;
	float *color;
	unsigned int i;
	GLsizei vert_size = 2 * sizeof(float);
	GLsizei color_size = 4 * sizeof(float);
	GLsizei vert_buffer_size = vert_size * (sv + count);
	GLsizei color_buffer_size = color_size * (sv + count);
	GLsizei buffer_size = vert_buffer_size + color_buffer_size;
	int quad_x1 = piglit_width / 2 - 5;
	int quad_y1 = piglit_height / 2 - 5;
	int quad_x2 = quad_x1 + 10;
	int quad_y2 = quad_y1 + 10;
	int smallquad_x1 = piglit_width / 2;
	int smallquad_y1 = piglit_height / 2;
	int smallquad_x2 = smallquad_x1 + 1;
	int smallquad_y2 = smallquad_y1 + 1;
	piglit_ortho_projection(piglit_width, piglit_height, false);

	if (use_vbo) {
		GLuint vbo;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STREAM_DRAW);
		glVertexPointer(2, GL_FLOAT, 0, 0);

		data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	} else {
		data = malloc(buffer_size);
	}
	if (!data)
		return PIGLIT_FAIL;

	vert = (float *)data;
	color = (float *)(data + vert_buffer_size);

	if (use_vbo) {
		glVertexPointer(2, GL_FLOAT, 0, 0);
		glColorPointer(4, GL_FLOAT, 0,
			       (void *)(uintptr_t)vert_buffer_size);
	} else {
		glVertexPointer(2, GL_FLOAT, 0, vert);
		glColorPointer(4, GL_FLOAT, 0, color);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glClear(GL_COLOR_BUFFER_BIT);

	/* Initialize all colors to red. */
	set_colors(color, 0, sv + count, red);

	/* Initialize all the vertices to offscreen. */
	for (i = 0; i < sv + count; i++)
		set_point(vert, i, -1, -1);

	switch (primtype) {
	case GL_POINTS:
		/* Four points in the middle of the screen. */
		v = sv + v_from_end(count, 4, 1);
		set_colors(color, v, 4, green);
		/* Adjust point and line coordinates to pixel centers,
		 * to prevent rounding-based test failures.
		 */
		set_point(vert, v++, smallquad_x1 + 0.5, smallquad_y1 + 0.5);
		set_point(vert, v++, smallquad_x2 + 0.5, smallquad_y1 + 0.5);
		set_point(vert, v++, smallquad_x1 + 0.5, smallquad_y2 + 0.5);
		set_point(vert, v++, smallquad_x2 + 0.5, smallquad_y2 + 0.5);
		break;

	case GL_LINES:
		/* Draw the outline of a quad. */
		v = sv + v_from_end(count, 4, 2);
		set_colors(color, v, 4 * 2, green);
		set_point(vert, v++, quad_x1, quad_y1 + 0.5);
		set_point(vert, v++, quad_x2, quad_y1 + 0.5);

		set_point(vert, v++, quad_x1, quad_y2 + 0.5);
		set_point(vert, v++, quad_x2, quad_y2 + 0.5);

		set_point(vert, v++, quad_x1 + 0.5, quad_y1);
		set_point(vert, v++, quad_x1 + 0.5, quad_y2);

		set_point(vert, v++, quad_x2 + 0.5, quad_y1);
		set_point(vert, v++, quad_x2 + 0.5, quad_y2);
		break;

	case GL_LINE_STRIP:
		v = sv + v_from_end(count, 4, 1);
		/* Strip start point plus a bunch of degenerate lines. */
		for (i = sv; i < v; i++)
			set_point(vert, i, quad_x1 + 0.5, quad_y1 + 0.5);

		/* Last 4 points producing the real lines. */
		set_colors(color, v - 1, 5, green);
		set_point(vert, v++, quad_x2 + 0.5, quad_y1 + 0.5);
		set_point(vert, v++, quad_x2 + 0.5, quad_y2 + 0.5);
		set_point(vert, v++, quad_x1 + 0.5, quad_y2 + 0.5);
		set_point(vert, v++, quad_x1 + 0.5, quad_y1 + 0.5);
		break;

	case GL_LINE_LOOP:
		v = sv + v_from_end(count, 3, 1);
		/* Loop start point plus a bunch of degenerate lines. */
		set_colors(color, sv, 1, green);
		for (i = sv; i < v; i++)
			set_point(vert, i, quad_x1 + 0.5, quad_y1 + 0.5);

		/* Last 3 points producing the real lines. */
		set_colors(color, v - 1, 4, green);
		set_point(vert, v++, quad_x2 + 0.5, quad_y1 + 0.5);
		set_point(vert, v++, quad_x2 + 0.5, quad_y2 + 0.5);
		set_point(vert, v++, quad_x1 + 0.5, quad_y2 + 0.5);
		break;

	case GL_TRIANGLES:
		/* Set up a pair of triangles to make a quad. */
		v = sv + v_from_end(count, 2, 3);
		set_colors(color, v, 2 * 3, green);
		set_point(vert, v++, quad_x1, quad_y1);
		set_point(vert, v++, quad_x2, quad_y1);
		set_point(vert, v++, quad_x1, quad_y2);

		set_point(vert, v++, quad_x2, quad_y1);
		set_point(vert, v++, quad_x2, quad_y2);
		set_point(vert, v++, quad_x1, quad_y2);
		break;

	case GL_TRIANGLE_STRIP:
		v = sv + v_from_end(count, 3, 1);
		/* A bunch of degenerate tri strip triangles. */
		for (i = sv; i < v; i++)
			set_point(vert, i, quad_x1, quad_y1);

		/* Last 3 strip points producing one more degenerate
		 * plus two real tris.
		 */
		set_colors(color, v - 1, 4, green);
		set_point(vert, v++, quad_x2, quad_y1);
		set_point(vert, v++, quad_x1, quad_y2);
		set_point(vert, v++, quad_x2, quad_y2);
		break;

	case GL_TRIANGLE_FAN:
	case GL_POLYGON:
		v = sv + v_from_end(count, 3, 1);
		/* Fan start point plus a bunch of degenerate tris. */
		set_colors(color, sv, 1, green);
		for (i = sv; i < v; i++)
			set_point(vert, i, quad_x1, quad_y1);

		/* Last 3 fan points producing the real tris. */
		set_colors(color, v, 3, green);
		set_point(vert, v++, quad_x2, quad_y1);
		set_point(vert, v++, quad_x2, quad_y2);
		set_point(vert, v++, quad_x1, quad_y2);
		break;

	case GL_QUADS:
		v = sv + v_from_end(count, 1, 4);
		set_colors(color, v, 4, green);
		set_point(vert, v++, quad_x1, quad_y1);
		set_point(vert, v++, quad_x2, quad_y1);
		set_point(vert, v++, quad_x2, quad_y2);
		set_point(vert, v++, quad_x1, quad_y2);
		break;

	case GL_QUAD_STRIP:
		v = sv + v_from_end(count, 1, 2);
		for (i = sv; i < v; i++) {
			if ((i - sv) % 2 == 0)
				set_point(vert, i, quad_x1, quad_y1);
			else
				set_point(vert, i, quad_x2, quad_y1);
		}
		set_colors(color, v - 2, 4, green);
		set_point(vert, v++, quad_x1, quad_y2);
		set_point(vert, v++, quad_x2, quad_y2);
		break;

	default:
		fprintf(stderr, "bad primitive\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	/* Initialize the 0 - sv primitives to something that would
	 * draw some red garbage if we were to accidentally draw using
	 * them.
	 */
	v = 0;
	switch (primtype) {
	case GL_POINTS:
		for (i = 0; i < sv; i++) {
			set_point(vert, v++,
				  i % piglit_width,
				  (i / piglit_width) % piglit_height);
		}
		break;

	case GL_LINES:
	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
		for (i = 0; i < sv / 2; i++) {
			int y = i % piglit_height;
			set_point(vert, v++, 0, y);
			set_point(vert, v++, piglit_width, y);
		}
		break;

	case GL_TRIANGLES:
	case GL_TRIANGLE_FAN:
	case GL_TRIANGLE_STRIP:
		for (i = 0; i < sv / 3; i++) {
			set_point(vert, v++, 0, 0);
			set_point(vert, v++, piglit_width, 0);
			set_point(vert, v++, 0, piglit_height);
		}
		break;

	case GL_QUADS:
	case GL_QUAD_STRIP:
	case GL_POLYGON:
		for (i = 0; i < sv / 4; i++) {
			set_point(vert, v++, 0, 0);
			set_point(vert, v++, piglit_width, 0);
			set_point(vert, v++, piglit_width, piglit_height);
			set_point(vert, v++, 0, piglit_height);
		}
		break;

	default:
		fprintf(stderr, "bad primitive\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	if (use_vbo)
		glUnmapBuffer(GL_ARRAY_BUFFER);

	glDrawArrays(primtype, sv, count);

	switch (primtype) {
	case GL_POINTS:
		pass = probe_rect(smallquad_x1, smallquad_y1,
				  smallquad_x2 - smallquad_x1 + 1,
				  smallquad_y2 - smallquad_y1 + 1);
		break;

	case GL_LINES:
	case GL_LINE_LOOP:
	case GL_LINE_STRIP:
		pass = probe_line_rect(quad_x1, quad_y1,
				       quad_x2, quad_y2);
		break;

	case GL_TRIANGLES:
	case GL_TRIANGLE_FAN:
	case GL_TRIANGLE_STRIP:
	case GL_QUADS:
	case GL_QUAD_STRIP:
	case GL_POLYGON:
		pass = probe_rect(quad_x1, quad_y1,
				  quad_x2 - quad_x1,
				  quad_y2 - quad_y1);
		break;

	default:
		fprintf(stderr, "bad primitive\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	piglit_present_results();

	if (!use_vbo)
		free(data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const struct {
	const char *name;
	GLenum e;
} prims[] = {
	{ "GL_POINTS", GL_POINTS },
	{ "GL_LINES", GL_LINES },
	{ "GL_LINE_STRIP", GL_LINE_STRIP },
	{ "GL_LINE_LOOP", GL_LINE_LOOP },
	{ "GL_TRIANGLES", GL_TRIANGLES },
	{ "GL_TRIANGLE_STRIP", GL_TRIANGLE_STRIP },
	{ "GL_TRIANGLE_FAN", GL_TRIANGLE_FAN },
	{ "GL_QUADS", GL_QUADS },
	{ "GL_QUAD_STRIP", GL_QUAD_STRIP },
	{ "GL_POLYGON", GL_POLYGON },
};

static void usage(const char *progname)
{
	int i;

	fprintf(stderr, "Usage: %s <vertcount> <vbo|varray> <primtype>\n",
		progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "primtype may be:\n");
	for (i = 0; i < ARRAY_SIZE(prims); i++)
		fprintf(stderr, "    %s\n", prims[i].name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	int i;

	if (argc != 4)
		usage(argv[0]);

	count = atoi(argv[1]);

	if (strcmp(argv[2], "vbo") == 0) {
		piglit_require_extension("GL_ARB_vertex_buffer_object");
		use_vbo = true;
	} else if (strcmp(argv[2], "varray") != 0) {
		usage(argv[0]);
	}

	for (i = 0; i < ARRAY_SIZE(prims); i++) {
		if (strcmp(prims[i].name, argv[3]) == 0) {
			primtype = prims[i].e;
			break;
		}
	}
	if (i == ARRAY_SIZE(prims))
		usage(argv[0]);
}
