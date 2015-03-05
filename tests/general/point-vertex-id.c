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

#include "piglit-util-gl.h"

/**
 * @file point-vertex-id.c
 *
 * Tests glPolygonMode(GL_POINT) used in combination with gl_VertexID
 * or gl_InstanceID or both.
 *
 * Specify gl_VertexID or gl_InstanceID as an argument to specify
 * which to test. Alternatively you can specify both in order to test
 * a combination of both.
 *
 * See bug #84677
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char
vertex_shader[] =
	"uniform vec2 viewport_size;\n"
	"\n"
	"#ifdef USE_VERTEX_ID\n"
	"uniform vec2 pos_array[12];\n"
	"#else\n"
	"in vec2 pos;\n"
	"#endif"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"#ifdef USE_VERTEX_ID\n"
	"        vec2 pos = pos_array[gl_VertexID];\n"
	"#endif\n"
	"        gl_Position = vec4(pos, 0.0, 1.0);\n"
	"#ifdef USE_INSTANCE_ID\n"
	"        gl_Position.t += float(gl_InstanceID) * 20.0;\n"
	"#endif\n"
	"        gl_Position.st = ((gl_Position.st + 0.5) * 2.0 /\n"
	"                          viewport_size - 1.0);\n"
	"        gl_FrontColor = vec4(1.0);\n"
	"}\n";

struct vertex {
	int x, y;
	GLubyte edge_flag;
};

enum test_mode_flags {
	TEST_MODE_VERTEX_ID = (1 << 0),
	TEST_MODE_INSTANCE_ID = (1 << 1),
};

static enum test_mode_flags test_modes;

static const struct vertex
vertices[] = {
	{ 10, 10, GL_TRUE },
	{ 20, 10, GL_TRUE },
	{ 10, 20, GL_TRUE },
	/* This triangle won't be drawn because none of the vertices
	 * are an edge */
	{ 30, 10, GL_FALSE },
	{ 40, 10, GL_FALSE },
	{ 30, 20, GL_FALSE },
	/* Copy of the above two triangles but shifted up by 20. If
	 * instanced rendering is used these will be generated based
	 * on the gl_InstanceID instead.
	 */
	{ 10, 30, GL_TRUE },
	{ 20, 30, GL_TRUE },
	{ 10, 40, GL_TRUE },
	{ 30, 30, GL_FALSE },
	{ 40, 30, GL_FALSE },
	{ 30, 40, GL_FALSE },
};

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	char shader_buf[sizeof vertex_shader + 512];
	GLint pos_location = 0, viewport_size_location;
	GLuint program;
	float *ref_image, *p;
	int i;

	strcpy(shader_buf, "#version 130\n");
	if (test_modes & TEST_MODE_INSTANCE_ID) {
		strcat(shader_buf,
		       "#extension GL_ARB_draw_instanced : require\n"
		       "#define USE_INSTANCE_ID\n");
	}
	if (test_modes & TEST_MODE_VERTEX_ID)
		strcat(shader_buf, "#define USE_VERTEX_ID\n");
	strcat(shader_buf, vertex_shader);

	program = piglit_build_simple_program(shader_buf, NULL);

	glUseProgram(program);

	glClear(GL_COLOR_BUFFER_BIT);

	viewport_size_location = glGetUniformLocation(program, "viewport_size");
	glUniform2f(viewport_size_location,
		    piglit_width,
		    piglit_height);

	if (test_modes & TEST_MODE_VERTEX_ID) {
		pos_location = glGetUniformLocation(program, "pos_array");

		for (i = 0; i < ARRAY_SIZE(vertices); i++) {
			glUniform2f(pos_location + i,
				    vertices[i].x,
				    vertices[i].y);
		}
	}

	glEnableClientState(GL_EDGE_FLAG_ARRAY);
	glEdgeFlagPointer(sizeof (struct vertex),
			  &vertices[0].edge_flag);

	if (!(test_modes & TEST_MODE_VERTEX_ID)) {
		pos_location = glGetAttribLocation(program, "pos");
		if (pos_location == -1)
			piglit_report_result(PIGLIT_FAIL);
		glEnableVertexAttribArray(pos_location);
		glVertexAttribPointer(pos_location,
				      2, /* size */
				      GL_INT,
				      GL_FALSE, /* normalized */
				      sizeof (struct vertex),
				      &vertices[0].x);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	if ((test_modes & TEST_MODE_INSTANCE_ID)) {
		glDrawArraysInstanced(GL_TRIANGLES,
				      0, /* first */
				      ARRAY_SIZE(vertices) / 2,
				      2 /* primcount */);
	} else {
		glDrawArrays(GL_TRIANGLES,
			     0, /* first */
			     ARRAY_SIZE(vertices));
	}

	if (!(test_modes & TEST_MODE_VERTEX_ID))
		glDisableVertexAttribArray(pos_location);

	ref_image = malloc(piglit_width * piglit_height * 3 *
			   sizeof (float));
	memset(ref_image, 0, piglit_width * piglit_height * 3 * sizeof (float));
	for (i = 0; i < ARRAY_SIZE(vertices); i++) {
		if (!vertices[i].edge_flag)
			continue;

		p = (ref_image +
		     (vertices[i].x + vertices[i].y * piglit_width) * 3);
		p[0] = p[1] = p[2] = 1.0f;
	}
	pass = piglit_probe_image_color(0, 0,
					piglit_width, piglit_height,
					GL_RGB,
					ref_image);

	glUseProgram(0);
	glDeleteProgram(program);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "gl_VertexID")) {
			test_modes |= TEST_MODE_VERTEX_ID;
		} else if (!strcmp(argv[i], "gl_InstanceID")) {
			test_modes |= TEST_MODE_INSTANCE_ID;
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (test_modes == 0) {
		fprintf(stderr,
			"usage: point-vertex-id [gl_VertexID] [gl_InstanceID]\n"
			"Either one or both of the arguments must be "
			"specified\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if ((test_modes & TEST_MODE_INSTANCE_ID))
		piglit_require_extension("GL_ARB_draw_instanced");

	piglit_require_GLSL_version(130);
}
