/*
 * Copyright © 2014 Intel Corporation
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
 * \file xfb-rendezvous_by_location.c
 * Verify that transform feedback data lands in the correct place when
 * rendezvous-by-location is used.
 *
 * Use a single vertex shader with outputs with non-contiguous explicit
 * locations.  Specify transform feedback with the vertex shader outputs
 * landing in a different order than the explicit locations specify.  Verify
 * that the order specified by glTransformFeedbackVaryings is used.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"layout(location = 1) out vec3 a;\n"
	"layout(location = 3) out vec3 b;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = piglit_vertex.xyz;\n"
	"    b = vec3(3, 5, 7);\n"
	"}\n"
	;

void piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	GLuint vs_prog = 0;
	GLuint buf = 0;
	GLuint xfb = 0;
	GLuint pipe = 0;
	char *source;
	float *data;
	unsigned i;
	bool pass = true;
	static const char *varyings[] = {"b", "a"};

	/* The vertex data is expected in this order because piglit_draw_rect
	 * uses a GL_TRIANGLE_STRIP to draw the rectangle.
	 */
	static const float expected_data[6 * 6] = {
		3, 5, 7, -1, -1, 0,
		3, 5, 7,  1, -1, 0,
		3, 5, 7, -1,  1, 0,
		3, 5, 7, -1,  1, 0,
		3, 5, 7,  1, -1, 0,
		3, 5, 7,  1,  1, 0
	};

	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_transform_feedback2");

	glsl_version = pick_a_glsl_version();

	/* The vertex shader must be created using the "traditional" method
	 * because we the call glTransformFeedbackVaryings before linking.
	 */
	asprintf(&source, vs_template, glsl_version);

	if (!CreateShaderProgram_with_xfb(source, varyings,
					  ARRAY_SIZE(varyings), &vs_prog)) {
		pass = false;
		goto done;
	}

	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);
	glUseProgramStages(pipe,
			   GL_VERTEX_SHADER_BIT,
			   vs_prog);

	configure_transform_feedback_object(&xfb, &buf);

	glEnable(GL_RASTERIZER_DISCARD);

	/* This will generate 4 vertices worth of transform feedback data.
	 */
	glBeginTransformFeedback(GL_TRIANGLES);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndTransformFeedback();

	/* Verify that the correct data landed in the correct places.
	 */
	data = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);

	for (i = 0; i < 6; i++) {
		if (memcmp(&data[i * 6], &expected_data[i * 6],
			   sizeof(float) * 6) != 0) {
			printf("Incorrect XFB data for veretx %d.  Got\n", i);
			printf("    %f %f %f %f %f %f\n",
			       data[(i * 6) + 0],
			       data[(i * 6) + 1],
			       data[(i * 6) + 2],
			       data[(i * 6) + 3],
			       data[(i * 6) + 4],
			       data[(i * 6) + 5]);
			printf("but expected\n");
			printf("    %f %f %f %f %f %f\n\n",
			       expected_data[(i * 6) + 0],
			       expected_data[(i * 6) + 1],
			       expected_data[(i * 6) + 2],
			       expected_data[(i * 6) + 3],
			       expected_data[(i * 6) + 4],
			       expected_data[(i * 6) + 5]);
		}
	}

	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	pass = piglit_check_gl_error(0) && pass;

 done:
	free(source);

	glBindProgramPipeline(0);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	glDeleteBuffers(1, &buf);
	glDeleteTransformFeedbacks(1, &xfb);
	glDeleteProgramPipelines(1, &pipe);
	glDeleteProgram(vs_prog);

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
