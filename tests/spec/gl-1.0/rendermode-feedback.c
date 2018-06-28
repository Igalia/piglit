/* Copyright © 2011 Intel Corporation
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

/**
 * @file rendermode-feedback.c
 *
 * Tests that glRenderMode(GL_FEEDBACK) rendering trivially works.
 */

static float vertex_array[] = {
	1.0, 2.0, 0.4, 1.0,
	3.0, 4.0, 0.6, 1.0,
	5.0, 6.0, 0.8, 1.0,
};

static float color_array[] = {
	0.01, 0.02, 0.03, 0.04,
	0.05, 0.06, 0.07, 0.08,
	0.09, 0.10, 0.11, 0.12,
};

static float texcoord_array[] = {
	101.0, 102.0, 103.0, 104.0,
	105.0, 106.0, 107.0, 108.0,
	109.0, 110.0, 111.0, 112.0,
};

static const float gl_2d_values[] =
	{GL_POLYGON_TOKEN, 3,
	 1.0, 2.0,
	 3.0, 4.0,
	 5.0, 6.0};

static const float gl_3d_values[] =
	{GL_POLYGON_TOKEN, 3,
	 1.0, 2.0, 0.3,
	 3.0, 4.0, 0.2,
	 5.0, 6.0, 0.1};

static const float gl_3d_color_values[] =
	{GL_POLYGON_TOKEN, 3,
	 1.0, 2.0, 0.3, 0.01, 0.02, 0.03, 0.04,
	 3.0, 4.0, 0.2, 0.05, 0.06, 0.07, 0.08,
	 5.0, 6.0, 0.1, 0.09, 0.10, 0.11, 0.12};

static const float gl_3d_color_texture_values[] =
	{GL_POLYGON_TOKEN, 3,
	 1.0, 2.0, 0.3, 0.01, 0.02, 0.03, 0.04, 101.0, 102.0, 103.0, 104.0,
	 3.0, 4.0, 0.2, 0.05, 0.06, 0.07, 0.08, 105.0, 106.0, 107.0, 108.0,
	 5.0, 6.0, 0.1, 0.09, 0.10, 0.11, 0.12, 109.0, 110.0, 111.0, 112.0};

static const float gl_4d_color_texture_values[] =
	{GL_POLYGON_TOKEN, 3,
	 1.0, 2.0, 0.3, 1.0, 0.01, 0.02, 0.03, 0.04, 101.0, 102.0, 103.0, 104.0,
	 3.0, 4.0, 0.2, 1.0, 0.05, 0.06, 0.07, 0.08, 105.0, 106.0, 107.0, 108.0,
	 5.0, 6.0, 0.1, 1.0, 0.09, 0.10, 0.11, 0.12, 109.0, 110.0, 111.0, 112.0};

struct type {
	GLenum type;
	const float *values;
	int count;
} types[] = {
	{ GL_2D, gl_2d_values, ARRAY_SIZE(gl_2d_values) },

	{ GL_3D, gl_3d_values, ARRAY_SIZE(gl_3d_values) },

	{ GL_3D_COLOR, gl_3d_color_values, ARRAY_SIZE(gl_3d_color_values) },

	{ GL_3D_COLOR_TEXTURE, gl_3d_color_texture_values,
	  ARRAY_SIZE(gl_3d_color_texture_values) },

	{ GL_4D_COLOR_TEXTURE, gl_4d_color_texture_values,
	  ARRAY_SIZE(gl_4d_color_texture_values) },
};

static enum piglit_result run_subtest(void * data);
static const struct piglit_gl_test_config * piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;

	struct piglit_subtest tests[ARRAY_SIZE(types) + 1];
	for (unsigned i = 0; i < ARRAY_SIZE(types); ++i) {
		tests[i].name = piglit_get_gl_enum_name(types[i].type);
		tests[i].option = tests[i].name;
		tests[i].subtest_func = run_subtest;
		tests[i].data = (void *)&types[i];
	}
	tests[ARRAY_SIZE(types)] = (struct piglit_subtest){ 0 };
	config.subtests = tests;

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static void
report_failure(struct type *type, float *buffer, int count)
{
	int i;

	fprintf(stderr, "Feeback failed for %s:\n",
		piglit_get_gl_enum_name(type->type));

	fprintf(stderr, "  Expected:    Observed: (%d/%d)\n",
		count, type->count);
	for (i = 0; i < types->count; i++) {
		fprintf(stderr, "  %9f    %9f\n", type->values[i], buffer[i]);
	}
	fprintf(stderr, "\n");
}

static enum piglit_result
run_subtest(void * data) {
	struct type * type = (struct type *)data;
	bool case_pass = true;
	int returned_count, j;
	const char *name = piglit_get_gl_enum_name(type->type);
	float buffer[2 +
		     ARRAY_SIZE(vertex_array) +
		     ARRAY_SIZE(color_array) +
		     ARRAY_SIZE(texcoord_array)];

	printf("Testing %s\n", name);

	for (j = 0; j < ARRAY_SIZE(buffer); j++)
		buffer[j] = -1.0;

	glFeedbackBuffer(ARRAY_SIZE(buffer), type->type, buffer);
	glRenderMode(GL_FEEDBACK);
	glDrawArrays(GL_TRIANGLES, 0, 4);
	returned_count = glRenderMode(GL_RENDER);

	if (returned_count != type->count) {
		case_pass = false;
	} else {
		for (j = 0; j < type->count; j++) {
			if (fabs(buffer[j] - type->values[j]) > .01)
				case_pass = false;
		}
	}

	if (!case_pass) {
		report_failure(type, buffer, returned_count);
	}
	return case_pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(4, GL_FLOAT, 0, vertex_array);
	glColorPointer(4, GL_FLOAT, 0, color_array);
	glTexCoordPointer(4, GL_FLOAT, 0, texcoord_array);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	result = piglit_run_selected_subtests(
		piglit_config->subtests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		result);

	piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_vertex_array");
}
