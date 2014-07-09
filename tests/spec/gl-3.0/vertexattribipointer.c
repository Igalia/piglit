/*
 * Copyright 2014 VMware, Inc.
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
 * Test OpenGL 3.0's glVertexAttribIPointer function with all combinations
 * of types and sizes.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


/**
 * We'll pass in the same data for both the signed and unsigned vertex
 * attributes and the signed/unsigned expected value.
 */
static const char *vertShaderText =
	"#version 130 \n"
	"uniform ivec4 expected_i; \n"
	"in ivec4 attr_i; \n"
	"uniform uvec4 expected_u; \n"
	"in uvec4 attr_u; \n"
	"out vec4 color; \n"
	" \n"
	"void main() \n"
	"{ \n"
	"   gl_Position = gl_Vertex; \n"
	"   if (attr_i == expected_i && attr_u == expected_u) \n"
	"      color = vec4(0, 1, 0, 0); // good! \n"
	"   else \n"
	"      color = vec4(1, 0, 0, 0); // bad! \n"
	"} \n";

static const char *fragShaderText =
	"#version 130 \n"
	"in vec4 color;\n"
	"void main()\n"
	"{ \n"
	"   gl_FragColor = color; \n"
	"} \n";


static const GLubyte ubyte4_data[] = { 100, 0, 200, 255 };
static const GLbyte byte4_data[] = { 50, 0, -25, -50 };
static const GLushort ushort4_data[] = { 16000, 0, 32000, 65535 };
static const GLshort short4_data[] = { 2000, 0, -4000, -32010 };
static const GLuint uint4_data[] = { 10000000, 0, 20000000, 80000020 };
static const GLint int4_data[] = { 10000000, 0, -20000000, -40000020 };

static GLuint Prog;
static GLint ExpectedUniform_i, ExpectedUniform_u;
static GLint Attr_i, Attr_u;


/*
 * Test glVertexAttribIArray(type, size, normalized)
 */
static bool
test_array(GLenum type, GLuint size)
{
	static const GLfloat verts[4][2] = {
		{ -1.0, -1.0 },
		{ 1.0, -1.0 },
		{ 1.0, 1.0 },
		{ -1.0, 1.0 }
	};
	static const GLfloat green[4] = { 0.0, 1.0, 0.0, 0.0 };
	GLubyte attr_buffer[4 * 4 * sizeof(GLuint)];
	int typeSize;
	const void *data;
	GLint expected[4];
	int i, p;

	switch (type) {
	case GL_BYTE:
		typeSize = sizeof(GLbyte);
		data = byte4_data;
		for (i = 0; i < 4; i++)
			expected[i] = byte4_data[i];
		break;
	case GL_UNSIGNED_BYTE:
		typeSize = sizeof(GLubyte);
		data = ubyte4_data;
		for (i = 0; i < 4; i++)
			expected[i] = ubyte4_data[i];
		break;
	case GL_SHORT:
		typeSize = sizeof(GLshort);
		data = short4_data;
		for (i = 0; i < 4; i++)
			expected[i] = short4_data[i];
		break;
	case GL_UNSIGNED_SHORT:
		typeSize = sizeof(GLushort);
		data = ushort4_data;
		for (i = 0; i < 4; i++)
			expected[i] = ushort4_data[i];
		break;
	case GL_INT:
		typeSize = sizeof(GLint);
		data = int4_data;
		for (i = 0; i < 4; i++)
			expected[i] = int4_data[i];
		break;
	case GL_UNSIGNED_INT:
		typeSize = sizeof(GLuint);
		data = uint4_data;
		for (i = 0; i < 4; i++)
			expected[i] = uint4_data[i];
		break;
	default:
		assert(0);
		typeSize = sizeof(GLint);
	}

	/* set unused components to defaults */
	switch (size) {
	case 1:
		expected[1] = 0;
	case 2:
		expected[2] = 0;
	case 3:
		expected[3] = 1;
	}

	/* Setup the attribute buffer by making four copies of the
	 * test's array data (for the four vertices).
	 */
	{
		int i, sz = typeSize * size;
		for (i = 0; i < 4; i++) {
			memcpy(attr_buffer + i * sz, data, sz);
		}		       
	}

	/* We set up both the signed and unsigned int attribute arrays to
	 * point to the same vertex data.
	 */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(Attr_i, size, type, 0, attr_buffer);
	glEnableVertexAttribArray(Attr_i);
	glVertexAttribIPointer(Attr_u, size, type, 0, attr_buffer);
	glEnableVertexAttribArray(Attr_u);

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(1,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* The same value is expected for the signed and unsigned attributes */
	glUniform4iv(ExpectedUniform_i, 1, expected);
	glUniform4uiv(ExpectedUniform_u, 1, (const GLuint *) expected);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(Attr_i);
	glDisableVertexAttribArray(Attr_u);

	p = piglit_probe_pixel_rgba(piglit_width / 2, piglit_height / 2,
				    green);
	if (!p) {
		printf("Test %s[%d] failed\n",
		       piglit_get_gl_enum_name(type), size);
                fflush(stdout);
	}

	piglit_present_results();

	return p;
}


enum piglit_result
piglit_display(void)
{
	static const GLenum types[] = {
		GL_BYTE,
		GL_UNSIGNED_BYTE,
		GL_SHORT,
		GL_UNSIGNED_SHORT,
		GL_INT,
		GL_UNSIGNED_INT
	};
	bool pass = true;
	int t, size;

	for (t = 0; t < ARRAY_SIZE(types); t++) {
		for (size = 1; size <= 4; size++) {
			pass = test_array(types[t], size) && pass;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);

	Prog = piglit_build_simple_program(vertShaderText, fragShaderText);
	if (!Prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(Prog);

	Attr_i = glGetAttribLocation(Prog, "attr_i");
	Attr_u = glGetAttribLocation(Prog, "attr_u");
	ExpectedUniform_i = glGetUniformLocation(Prog, "expected_i");
	ExpectedUniform_u = glGetUniformLocation(Prog, "expected_u");
}
