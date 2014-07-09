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

/** \file overlapping-locations-input-attribs.c
 * From OpenGL 4.0 spec, page 61:
 *
 *     "It is possible for an application to bind more than one
 *     attribute name to the same location. This is referred to as
 *     aliasing. This will only work if only one of the aliased
 *     attributes is active in the executable program, or if no
 *     path through the shader consumes more than one attribute of
 *     a set of attributes aliased to the same location. A link
 *     error can occur if the linker determines that every path
 *     through the shader consumes multiple aliased attributes,
 *     but implementations are not required to generate an error
 *     in this case."
 *
 *     Aliasing of vertex input variables is allowed in: OpenGL
 *     2.0 (and above) and OpenGL ES 2.0. This test varifies
 *     that aliasing can be used successfully in a vertex shader
 *     under the constraints defined in OpenGL spec.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool locations_in_shader;
static unsigned prog, vao, vertex_buf;

void
compile_shader(void)
{
	GLuint element_buf;
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
	float vertex_data[4][11] = {
		/* vertex     color0:green    color1:blue     color2:yellow */
		{-1.0, -1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0, 0.0},
		{-1.0,  1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0, 0.0},
		{ 1.0,  1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0, 0.0},
		{ 1.0, -1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0, 0.0}};

	static const char *vert_template =
		"#version 130\n"
		"%s\n"
		"out vec4 color;\n"
		"uniform int x;\n"
		"void main()\n"
		"{\n"
		"  gl_Position =vec4(vertex, 0, 1);\n"
		"  switch(x) {\n"
		"  case 0:\n"
		"    color =vec4(color0, 1.0);\n"
		"    break;\n"
		"  case 1:\n"
		"    color = vec4(color1, 1.0);\n"
		"    break;\n"
		"  case 2:\n"
		"    color = vec4(color2, 1.0);\n"
		"    break;\n"
		"  default:\n"
		"    color = vec4(1.0);;\n"
		"  }\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"in vec4 color;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"    out_color = color;\n"
		"}\n";

	char *vert;
	asprintf(&vert, vert_template, locations_in_shader ?
		 "#extension GL_ARB_explicit_attrib_location : require\n"
		 "layout (location = 0) in vec2 vertex;\n"
		 "layout (location = 1) in vec3 color0;\n"
		 "layout (location = 1) in vec3 color1;\n"
		 "layout (location = 1) in vec3 color2;\n" :
		 "in vec2 vertex;\n"
		 "in vec3 color0;\n"
		 "in vec3 color1;\n"
		 "in vec3 color2;\n");

	prog = piglit_build_simple_program_unlinked(vert, frag);
	if (!locations_in_shader) {
		glBindAttribLocation(prog, 0, "vertex");
		glBindAttribLocation(prog, 1, "color0");
		glBindAttribLocation(prog, 1, "color1");
		glBindAttribLocation(prog, 1, "color2");
	}
	glLinkProgram(prog);

	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float),
			      (void *) 0);
	glEnableVertexAttribArray(1);

	/* Set up element input buffer to tesselate a quad into
	 * triangles
	 */
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <set_location>\n"
	       "  where <set_location> is one of:\n"
	       "    shader: set locations of input variables in shader program\n"
	       "    api: set locations of input variables using api\n",
	       prog_name);

	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);

	/* 1st arg: location */
	if(strcmp(argv[1], "shader") != 0 &&
	   strcmp(argv[1], "api") != 0)
		print_usage_and_exit(argv[0]);

	locations_in_shader = strcmp(argv[1], "shader") == 0;

	if (locations_in_shader)
		piglit_require_extension("GL_ARB_explicit_attrib_location");

	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_GLSL_version(130);
	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display()
{
	int i;
	bool pass = true;
	float expected[3][4] = {
		{0.0, 1.0, 0.0, 1.0}, /* green */
		{0.0, 0.0, 1.0, 1.0}, /* blue */
		{1.0, 1.0, 0.0, 1.0}}; /* yellow */

	glUseProgram(prog);

	for(i = 0; i < 3; i++) {
		glUniform1i(glGetUniformLocation(prog, "x"), i);

		/* Setup VertexAttribPointer for location=1. There should be
		 * only one active attribute pointer set to the shared location
		 * '1' at a time.
		 */
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float),
				      (void *) ((2 + 3*i) * sizeof(float)));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
		pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
					      expected[i]) && pass;
		piglit_present_results();
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
