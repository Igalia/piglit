/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * Triangle Rasterization Overdraw Test
 *
 * Draws a triangle fan to fill the screen and ensures every pixel was drawn only once
 * Based on idea from Brian Paul
 *
 *
 * Contains two methods of drawing to cover both clipped and unclipped triangles:
 *
 *   1. No-Clip: Picks a random point in the window and walks the perimeter of the window
 *               adding vertices to the triangle fan at non integer steps.
 *
 *   2. Clip:    Picks a random point in the window and adds vertices to the triangle fan
 *               around a circle that contains the entire window, thus going off screen.
 */

#include "piglit-util-gl.h"
#include "mersenne.hpp"

#include <time.h>
#include <vector>
#include <algorithm>

/* Data structures */
struct Vector
{
	Vector()
		: x(0), y(0)
	{
	}

	Vector(float x, float y)
		: x(x), y(y)
	{
	}

	float x, y;
};

/* Command line arguments */
bool rect = false;
bool clips = false;
bool break_on_fail = false;
int random_test_count = 10;

/* Piglit variables */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 1000;
	config.window_height = 1000;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Globals */
int test_id = 0;
Mersenne mersenne;


/* Random floating point number between 0 and 1. */
static inline float
random_float(void)
{
	const int float_range = 1 << 23;
	return (mersenne.value() % float_range) * (1.0 / float_range);
}

/* Random float from [a to b) */
static inline float
random_float(float a, float b)
{
	return a + (b - a - 1) * random_float();
}


struct TestCase
{
	Vector mid;
	std::vector<Vector> triangle_fan;

	struct {
		int x;
		int y;
		int w;
		int h;
	} probe_rect;

	void generate(void);
	bool run(void) const;
};


/* Generates a random triangle fan with a random origin, and contouring a rectangle or circle */
void TestCase::generate(void)
{
	/* Random center point */
	if (clips) {
		mid.x = random_float(-0.5*piglit_width, 1.5*piglit_width);
		mid.y = random_float(-0.5*piglit_height, 1.5*piglit_height);
	} else {
		mid.x = random_float(0, piglit_width);
		mid.y = random_float(0, piglit_height);
	}

	if (clips) {
		probe_rect.x = 0;
		probe_rect.y = 0;
		probe_rect.w = piglit_width;
		probe_rect.h = piglit_height;
	} else {
		probe_rect.x = piglit_width/4;
		probe_rect.y = piglit_height/4;
		probe_rect.w = piglit_width/2;
		probe_rect.h = piglit_height/2;
	}

	triangle_fan.clear();
	triangle_fan.push_back(mid);

	if (rect) {
		/* Step around the window perimeter adding triangles */
		double perimeter = probe_rect.w*2 + probe_rect.h*2;
		double pos = 0.0;

		while (pos < perimeter) {
			Vector vertex;

			if (pos < probe_rect.w) {
				/* bottom */
				vertex.x = probe_rect.x + pos;
				vertex.y = probe_rect.y + 0.0f;
			} else if (pos < probe_rect.w + probe_rect.h) {
				/* right */
				vertex.x = probe_rect.x + probe_rect.w;
				vertex.y = probe_rect.y + pos - probe_rect.w;
			} else if(pos < probe_rect.w*2 + probe_rect.h) {
				/* top */
				vertex.x = probe_rect.x + probe_rect.w - (pos - (probe_rect.w + probe_rect.h));
				vertex.y = probe_rect.y + probe_rect.h;
			} else {
				/* left */
				vertex.x = probe_rect.x + 0.0f;
				vertex.y = probe_rect.y + probe_rect.h - (pos - (probe_rect.w*2 + probe_rect.h));
			}

			triangle_fan.push_back(vertex);
			pos += random_float();
		}
	} else {
		/* Step around a circle that contains the window */
		double radius = (sqrt((double)(probe_rect.w*probe_rect.w + probe_rect.h*probe_rect.h)) / 2.0) + 5.0;
		double perimeter = 2.0 * M_PI * radius;
		double pos = 0.0;

		while (pos < perimeter) {
			double theta = pos / radius;

			float x = probe_rect.x + 0.5 * probe_rect.w + cos(theta) * radius;
			float y = probe_rect.y + 0.5 * probe_rect.h + sin(theta) * radius;

			triangle_fan.push_back(Vector(x, y));
			pos += random_float();
		}
	}

	/* Complete the fan! */
	triangle_fan.push_back(triangle_fan.at(1));
	++test_id;
}

/* Tests a triangle fan */
bool TestCase::run(void) const
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Set render state */
	float colour[4];
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	if (rect) {
		glBlendFunc(GL_ONE, GL_ONE);
		colour[0] = colour[1] = colour[2] = 127.0f / 255.0f;
	} else {
		/* Invert.
		 *
		 * When contouring a circle with very small steps, some overdraw occurs
		 * naturally, but it should cancel itself out, i.e., there should be an
		 * odd number of overdraw inside the shape, and an even number outside.
		 * */
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		colour[0] = colour[1] = colour[2] = 1.0f;
	}

	colour[3] = 1.0f;
	glColor4fv(colour);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw triangle fan */
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &triangle_fan.front());
	glDrawArrays(GL_TRIANGLE_FAN, 0, triangle_fan.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	/* Reset draw state */
	glDisable(GL_BLEND);

	if (!piglit_probe_rect_rgb(probe_rect.x, probe_rect.y, probe_rect.w, probe_rect.y, colour)) {
		printf("%d. Triangle Fan with %d triangles around (%f, %f)\n",
		       test_id, (int)triangle_fan.size(), mid.x, mid.y);

		fflush(stdout);
		return false;
	}

	return true;
}

/* Render */
enum piglit_result
piglit_display(void)
{
	/* Perform test */
	GLboolean pass = GL_TRUE;

	TestCase test_case;

	if (piglit_automatic) {
		int fail_count = 0;

		printf("Running %d random tests\n", random_test_count);
		fflush(stdout);

		for (int i = 0; i < random_test_count && !(fail_count && break_on_fail); ++i) {
			test_case.generate();
			if (!test_case.run())
				fail_count++;
		}

		printf("Failed %d random tests\n", fail_count);
		fflush(stdout);

		if (fail_count)
			pass = GL_FALSE;
	} else {
		test_case.generate();
		pass = pass && test_case.run();

		piglit_present_results();
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

/* Read command line arguments! */
void
piglit_init(int argc, char **argv)
{
	uint32_t seed = 0xfacebeef ^ time(NULL);

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-break_on_fail") == 0){
			break_on_fail = true;
			printf("Execution will stop on first fail\n");
		} else if (strcmp(argv[i], "-rect") == 0){
			rect = true;
		} else if (strcmp(argv[i], "-max_size") == 0){
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &piglit_width);
			piglit_height = piglit_width;
		} else if (strcmp(argv[i], "-clip") == 0){
			clips = true;
			printf("Clipped triangles are being tested\n");
		} else if (i + 1 < argc) {
			if (strcmp(argv[i], "-count") == 0) {
				random_test_count = strtoul(argv[++i], NULL, 0);
			} else if (strcmp(argv[i], "-seed") == 0) {
				seed = strtoul(argv[++i], NULL, 0);
			}
		}
	}

	printf("Random seed: 0x%08X\n", seed);
	mersenne.init(seed);
}
