/*
 * Copyright © 2015 Glenn Kennard
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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
 */

#include "common.h"

static int prog;

/* Note: meaningful test cases (with non-zero values) for the following are
 * missing:
 *  - GL_COMPUTE_SHADER_INVOCATIONS_ARB
 *  - GL_GEOMETRY_SHADER_INVOCATIONS
 *  - GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB
 *  - GL_TESS_CONTROL_SHADER_PATCHES_ARB
 *  - GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB
 *  - GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
 */
const struct query_type_desc query_types[] = {
	{ GL_ANY_SAMPLES_PASSED,			{ "GL_ARB_occlusion_query2", NULL } },
	{ GL_ANY_SAMPLES_PASSED_CONSERVATIVE,		{ "GL_ARB_ES3_compatibility", NULL } },
	{ GL_CLIPPING_INPUT_PRIMITIVES_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_COMPUTE_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", "GL_ARB_compute_shader" } },
	{ GL_FRAGMENT_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_GEOMETRY_SHADER_INVOCATIONS,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB,	{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_PRIMITIVES_GENERATED,			{ NULL, } },
	{ GL_PRIMITIVES_SUBMITTED_ARB,			{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_SAMPLES_PASSED_ARB,			{ NULL, } },
	{ GL_TESS_CONTROL_SHADER_PATCHES_ARB,		{ "GL_ARB_pipeline_statistics_query", "GL_ARB_tessellation_shader" } },
	{ GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB,	{ "GL_ARB_pipeline_statistics_query", "GL_ARB_tessellation_shader" } },
	{ GL_TIMESTAMP,					{ "GL_ARB_timer_query", NULL } },
	{ GL_TIME_ELAPSED,				{ "GL_ARB_timer_query", NULL } },
	{ GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,	{ NULL, } },
	{ GL_VERTEX_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_VERTICES_SUBMITTED_ARB,			{ "GL_ARB_pipeline_statistics_query", NULL } },
};

unsigned
num_query_types() {
	return ARRAY_SIZE(query_types);
}

void
get_query_values(const struct query_type_desc *desc, bool *exact, uint32_t *expected)
{
	*exact = true;

	switch (desc->type) {
	case GL_ANY_SAMPLES_PASSED:
	case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
		*expected = 1;
		break;
	case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
	case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_PRIMITIVES_GENERATED:
	case GL_PRIMITIVES_SUBMITTED_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_SAMPLES_PASSED_ARB:
		*expected = piglit_width * piglit_height;
		break;
	case GL_TIMESTAMP:
	case GL_TIME_ELAPSED:
		*exact = false;
		*expected = 1;
		break;
	case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
		*expected = 0;
		break;
	case GL_VERTEX_SHADER_INVOCATIONS_ARB:
	case GL_VERTICES_SUBMITTED_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
	case GL_GEOMETRY_SHADER_INVOCATIONS:
	case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
	case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
	case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
		*expected = 0;
		break;
	default:
		abort();
	}
}

bool
is_query_supported(const struct query_type_desc *desc)
{
	for (unsigned i = 0; i < ARRAY_SIZE(desc->extensions); ++i) {
		if (!desc->extensions[i])
			break;

		if (!piglit_is_extension_supported(desc->extensions[i]))
			return false;
	}

	return true;
}

void
run_query(unsigned query, const struct query_type_desc *desc)
{
	GLenum query_type = desc->type;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable query, draw something that should pass */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(prog);
	if (query_type != GL_TIMESTAMP)
		glBeginQuery(query_type, query);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2);
	if (query_type != GL_TIMESTAMP)
		glEndQuery(query_type);
	else
		glQueryCounter(query, query_type);
}

void
query_common_init()
{
	static const char vsCode[] =
		"#version 150\n"
		"in vec4 pos_in;\n"
		"void main() {\n"
		"	gl_Position = pos_in;\n"
		"}\n";

	static const char fsCode[] =
		"#version 150\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	color = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n";

	prog = piglit_build_simple_program(vsCode, fsCode);
}
