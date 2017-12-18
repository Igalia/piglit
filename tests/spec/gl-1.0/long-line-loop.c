/*
 * Copyright © 2017 Fabian Bieler
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
 * @file long-line-loop.c
 * Draw cricles with line loops and a line strips blended on top of each
 * other and check that the renderings match.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_width = 1024;
	config.window_height = 1024;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static int max_vertices;
static int num_vertices;
static int probe_location[2];
static const float radius = 0.9;

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s [<vertex_count>]\n"
	       "  where <vertex_count> is the number of vertices to test.\n"
	       "\n"
	       "  If omitted, sequentially test from 16 to max_vertices by "
	       "quadrupling,\n"
	       "  where max_vertices is GL_MAX_ELEMENTS_VERTICES clamped to "
	       "[0x10000, 0x40000].\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	glBlendFunc(GL_ONE, GL_ONE);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	if (argc == 1) {
		/* This isn't a hard limit, but staying below should help
		 * performance.
		 *
		 * XXX: It could be interesting to go beyond this limit to
		 * test a different code path in the GL implementation.
		 */
		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_vertices);
		max_vertices = CLAMP(max_vertices, 0x10000, 0x40000);
	} else if (argc == 2) {
		char *endptr;
		num_vertices = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
		if (num_vertices < 6)
			print_usage_and_exit(argv[0]);
	} else {
		print_usage_and_exit(argv[0]);
	}
}

static void
draw_circle(int segments)
{
	/* The first (segments - 1) vertices describe the arc of a circle
	 * slice with central angle (360° - alpha).
	 * The last vertex is identical to the first vertex.
	 * alpha is chosen so that the last line segment covers two pixels.
	 */
	const float alpha = asinf(2.0 / (piglit_width / 2 * radius));
	struct {
		float pos[4];
		float green[4];
		float blue[4];
	} *vertex = malloc(sizeof(vertex[0]) * segments);

	for (int i = 0; i < segments - 1; ++i) {
		const float phi = alpha -
				  (2 * M_PI - alpha) /
				  (float)(segments - 2) *
				  (float)i;

		vertex[i].pos[0] = round(piglit_width / 2 *
					  (1 + radius * cosf(phi))) + 0.5;
		vertex[i].pos[1] = round(piglit_height / 2 *
					  (1 + radius * sinf(phi))) + 0.5;
		vertex[i].pos[2] = 0;
		vertex[i].pos[3] = 1;
		vertex[i].green[0] = 0;
		vertex[i].green[1] = 1;
		vertex[i].green[2] = 0;
		vertex[i].green[3] = 1;
		vertex[i].blue[0] = 0;
		vertex[i].blue[1] = 0;
		vertex[i].blue[2] = 1;
		vertex[i].blue[3] = 1;
	}
	memcpy(&vertex[segments - 1], &vertex[0], sizeof(vertex[0]));

	/* Find a pixel in the last line segment: */
	for (int i = 0; i < 2; ++i)
		probe_location[i] = round((vertex[segments - 2].pos[i] +
					   vertex[segments - 1].pos[i] -
					   1.0) / 2.0);

	/* Render twice: */
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(4, GL_FLOAT, sizeof(vertex[0]), vertex[0].pos);
	glColorPointer(4, GL_FLOAT, sizeof(vertex[0]), vertex[0].green);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_LINE_LOOP, 0, segments - 1);

	glEnable(GL_BLEND);
	glColorPointer(4, GL_FLOAT, sizeof(vertex[0]), vertex[0].blue);
	glDrawArrays(GL_LINE_STRIP, 0, segments);
	glDisable(GL_BLEND);

	free(vertex);

	piglit_present_results();
}

static bool
check_circle(void)
{
	bool pass = true;
	const float teal[] = {0, 1, 1};
	const float black[] = {0, 0, 0};

	/* check that the two renderings are identical */
	pass = piglit_probe_rect_two_rgb(0, 0, piglit_width, piglit_height,
					 black, teal) && pass;

	/* belt + suspenders: Additionally check that the last line segment
	 * was drawn...
	 */
	pass = piglit_probe_pixel_rgb(probe_location[0], probe_location[1],
				      teal) && pass;

	/* ...and that the center of the circle is black */
	const int x = ceil(piglit_width / 2 * radius / M_SQRT2) + 1;
	const int y = ceil(piglit_height / 2 * radius / M_SQRT2) + 1;
	pass = piglit_probe_rect_rgb(x, y, piglit_width - 2 * x,
				     piglit_height - 2 * y, black) && pass;

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	if (max_vertices) {
		for (int vertices = 16;
		     vertices <= max_vertices;
		     vertices <<= 2) {
			draw_circle(vertices);
			pass = check_circle() && pass;
		}
	} else {
		draw_circle(num_vertices);
		pass = check_circle() && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

