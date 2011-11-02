/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 * EXT_transform_feedback test.
 *
 * Test that writing a variable with a specific GLSL type into a TFB buffer
 * works as expected.
 */

#include "piglit-util.h"

int piglit_width = 64;
int piglit_height = 32;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA;

struct test_desc {
	const char *name;
	const char *vs;
	unsigned num_varyings;
	const char *varyings[16];
	unsigned num_elements;
	float expected[256];
} tests[] = {
	{
		"float", /* name */

		"#version 110\n"
		"varying float r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = 666.0;"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		1, /* num_elements, expected */
		{666}
	},
	{
		"float[2]", /* name */

		"#version 120\n"
		"varying float r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](666.0, 0.123);"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		2, /* num_elements, expected */
		{666, 0.123}
	},
	{
		"vec2", /* name */

		"#version 110\n"
		"varying vec2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec2(666.0, 999.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		2, /* num_elements, expected */
		{666, 999}
	},
	{
		"vec2[2]", /* name */

		"#version 120\n"
		"varying vec2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec2[2](vec2(666.0, 999.0), vec2(-1.5, -20.0));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		4, /* num_elements, expected */
		{666, 999, -1.5, -20.0}
	},
	{
		"vec3", /* name */

		"#version 110\n"
		"varying vec3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec3(666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		3, /* num_elements, expected */
		{666, 999, -2}
	},
	{
		"vec3[2]", /* name */

		"#version 120\n"
		"varying vec3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec3[2](vec3(666.0, 999.0, -2.0), vec3(0.4, 1.4, 3.5));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		6, /* num_elements, expected */
		{666, 999, -2, 0.4, 1.4, 3.5}
	},
	{
		"vec4", /* name */

		"#version 110\n"
		"varying vec4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec4(0.666, 666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		4, /* num_elements, expected */
		{0.666, 666, 999, -2}
	},
	{
		"vec4[2]", /* name */

		"#version 120\n"
		"varying vec4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec4[2](vec4(0.666, 666.0, 999.0, -2.0), vec4(0.5, -0.4, 30.0, 40.0));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		8, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.5, -0.4, 30.0, 40.0}
	},
	{
		"mat2", /* name */

		"#version 110\n"
		"varying mat2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2(0.666, 666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		4, /* num_elements, expected */
		{0.666, 666, 999, -2}
	},
	{
		"mat2[2]", /* name */

		"#version 120\n"
		"varying mat2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2[2](mat2(0.666, 666.0, 999.0, -2.0),"
		"              mat2(0.34, 0.65, 0.14, -0.97));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		8, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.34, 0.65, 0.14, -0.97}
	},
	{
		"mat2x3", /* name */

		"#version 120\n"
		"varying mat2x3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x3(0.666, 666.0, 999.0, -2.0, 0.5, -0.4);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		6, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.5, -0.4}
	},
	{
		"mat2x4", /* name */

		"#version 120\n"
		"varying mat2x4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x4(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		8, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}
	},
	{
		"mat3x2", /* name */

		"#version 120\n"
		"varying mat3x2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x2(0.666, 666.0, 999.0,"
		"           -2.0, 0.2, 5.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		6, /* num_elements, expected */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0}
	},
	{
		"mat3", /* name */

		"#version 110\n"
		"varying mat3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3(0.666, 666.0, 999.0,"
		"           -2.0, 0.2, 5.0,"
		"           3.0, 0.3, -10.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		9, /* num_elements, expected */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0}
	},
	{
		"mat3x4", /* name */

		"#version 120\n"
		"varying mat3x4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x4(0.666, 666.0, 999.0,"
		"             -2.0, 0.2, 5.0,"
		"             3.0, 0.3, -10.0,"
		"             0.4, -4.1, -5.9);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		12, /* num_elements, expected */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9}
	},
	{
		"mat4x2", /* name */

		"#version 120\n"
		"varying mat4x2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x2(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		8, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}
	},
	{
		"mat4x3", /* name */

		"#version 120\n"
		"varying mat4x3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x3(0.666, 666.0, 999.0, -2.0,"
		"             0.5, -0.4, 30.0, 40.0,"
		"             0.3, 0.2, 0.1, 0.4);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		12, /* num_elements, expected */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4}
	},
	{
		"mat4", /* name */

		"#version 110\n"
		"varying mat4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4(0.666, 666.0, 999.0, -2.0,"
		"           0.2, 5.0, 3.0, 0.3,"
		"           -10.0, 20.1, 52.4, -34.3,"
		"           45.0, 56.0, 67.0, 78.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		16, /* num_elements, expected */
		{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0}
	},

	{NULL}
};
struct test_desc *test;

GLuint buf;
GLuint prog;

#define NUM_VERTICES 3
#define DEFAULT_VALUE 0.123456

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	unsigned i;
	float *data;

	/* Parse params. */
	for (i = 1; i < argc; i++) {
		struct test_desc *t;

		for (t = tests; t->name; t++) {
			if (!strcmp(argv[i], t->name)) {
				test = t;
				goto test_ready;
			}
		}
		fprintf(stderr, "Unknown test name.\n");
		exit(1);
	}
	test = &tests[0];
test_ready:

	printf("Testing type: %s\n", test->name);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	if (!GLEW_VERSION_1_5) {
		fprintf(stderr, "OpenGL 1.5 required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	piglit_require_GLSL();
	piglit_require_extension("GL_EXT_transform_feedback");

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, test->vs);
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	glTransformFeedbackVaryingsEXT(prog, test->num_varyings,
				       test->varyings,
				       GL_INTERLEAVED_ATTRIBS_EXT);
	piglit_LinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up the transform feedback buffer. */
	data = malloc(test->num_elements*NUM_VERTICES*sizeof(float));
	for (i = 0; i < test->num_elements*NUM_VERTICES; i++) {
		data[i] = DEFAULT_VALUE;
	}

	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
		     test->num_elements*NUM_VERTICES*sizeof(float),
		     data, GL_STREAM_READ);

	assert(glGetError() == 0);

	glBindBufferBaseEXT(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);

	assert(glGetError() == 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);

	free(data);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float *ptr;
	unsigned i;
	static const float verts[NUM_VERTICES*2] = {
		10, 10,
		10, 20,
		20, 20
	};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into TFBO. */
	glLoadIdentity();
	piglit_UseProgram(prog);
	glBeginTransformFeedbackEXT(GL_TRIANGLES);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
	glEndTransformFeedbackEXT();

	assert(glGetError() == 0);

	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	for (i = 0; i < test->num_elements*NUM_VERTICES; i++) {
		float value = test->expected[i % test->num_elements];

		if (fabs(ptr[i] - value) > 0.01) {
			printf("Buffer[%i]: %f,  Expected: %f\n", i, ptr[i], value);
			pass = GL_FALSE;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);

	assert(glGetError() == 0);

	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
