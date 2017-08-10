/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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
 * Test glVertexAttribPointer with size 3 and various combinations of types and
 * normalized/unnormalized, sourcing from a tightly-sized VBO. This exercises
 * a bounds checking boundary case.
 *
 * This failed with radeonsi on VI.
 *
 * Based on vertexattribpointer.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const char *vertShaderText =
	"uniform vec4 expected; \n"
	"uniform float tolerance; \n"
	"attribute vec4 attr; \n"
	"varying vec4 color; \n"
	" \n"
	"void main() \n"
	"{ \n"
	"   gl_Position = gl_Vertex; \n"
	"   vec4 diff = abs(expected - attr); \n"
	"   if (any(greaterThan(diff, vec4(tolerance)))) \n"
	"      color = vec4(1, attr.xyz); // bad! \n"
	"   else \n"
	"      color = vec4(0, 1, 0, 0); // good! \n"
	"} \n";

static const char *fragShaderText =
	"varying vec4 color;\n"
	"void main()\n"
	"{ \n"
	"   gl_FragColor = color; \n"
	"} \n";


static const GLfloat float3_data[] = { -0.5, 0.0, 0.75 };
static const GLubyte ubyte3_data[] = { 100, 0, 200 };
static const GLbyte byte3_data[] = { 50, 0, -25 };
static const GLushort ushort3_data[] = { 16000, 0, 32000 };
static const GLshort short3_data[] = { 2000, 0, -4000 };
static const GLuint uint3_data[] = { 10000000, 0, 20000000 };
static const GLint int3_data[] = { 10000000, 0, -20000000 };

static GLuint Prog;
static GLint ToleranceUniform, ExpectedUniform, AttrAttrib;


/*
 * Test glVertexAttribArray(type, normalized)
 */
static bool
test_array(GLenum type, GLboolean normalized)
{
	static const GLfloat verts[4][2] = {
		{ -1.0, -1.0 },
		{ 1.0, -1.0 },
		{ 1.0, 1.0 },
		{ -1.0, 1.0 }
	};
	static const GLfloat green[4] = { 0.0, 1.0, 0.0, 0.0 };
	float maxVal;
	int typeSize;
	const void *data;
	float expected[4];
	int i, p;
	float tolerance;
	GLuint vbo;

	switch (type) {
	case GL_BYTE:
		maxVal = 127.0;
		typeSize = sizeof(GLbyte);
		data = byte3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) byte3_data[i];
		break;
	case GL_UNSIGNED_BYTE:
		maxVal = 255.0;
		typeSize = sizeof(GLubyte);
		data = ubyte3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) ubyte3_data[i];
		break;
	case GL_SHORT:
		maxVal = 32767.0;
		typeSize = sizeof(GLshort);
		data = short3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) short3_data[i];
		break;
	case GL_UNSIGNED_SHORT:
		maxVal = 65535.0;
		typeSize = sizeof(GLushort);
		data = ushort3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) ushort3_data[i];
		break;
	case GL_INT:
		maxVal = (float) 0x7fffffff;
		typeSize = sizeof(GLint);
		data = int3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) int3_data[i];
		break;
	case GL_UNSIGNED_INT:
		maxVal = (float) 0xffffffff;
		typeSize = sizeof(GLuint);
		data = uint3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) uint3_data[i];
		break;
	case GL_FLOAT:
		maxVal = 1.0;
		typeSize = sizeof(GLfloat);
		data = float3_data;
		for (i = 0; i < 3; i++)
			expected[i] = (float) float3_data[i];
		break;
	default:
		abort();
	}

	if (normalized) {
		for (i = 0; i < 3; i++) {
			expected[i] /= maxVal;
		}
	}
	expected[3] = 1.0;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(AttrAttrib);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);

	/* Setup the attribute buffer by making four copies of the
	 * test's array data (for the four vertices).
	 *
	 * Use 4 * element_size as the stride; this increases the chances of
	 * staying on the fast path.
	 */
	{
		char buf[4 * 4 * sizeof(float)];
		unsigned vec_size = 3 * typeSize;
		unsigned stride = 4 * typeSize;
		unsigned buffer_size = 3 * stride + vec_size;

		assert(buffer_size < sizeof(buf));

		memset(buf, 0, sizeof(buf));
		for (unsigned i = 0; i < 4; i++)
			memcpy(buf + i * stride, data, vec_size);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, buffer_size, buf, GL_STATIC_DRAW);
		glVertexAttribPointer(AttrAttrib, 3, type, normalized, stride, NULL);
	}

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(1,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (type == GL_FLOAT ||
            type == GL_DOUBLE ||
            type == GL_INT ||
            type == GL_UNSIGNED_INT)
		tolerance = 1.0 / 0xffffff;  /* 1 / (2^24-1) */
	else
		tolerance = 1.0 / maxVal;

	glUniform1f(ToleranceUniform, tolerance);
	glUniform4fv(ExpectedUniform, 1, expected);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(AttrAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);

	p = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);
	if (!p) {
		printf("Test %s %s failed\n",
		       piglit_get_gl_enum_name(type),
		       (normalized ? "Normalized" : "Unnormalized"));
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
		GL_UNSIGNED_INT,
		GL_FLOAT,
	};
	bool pass = true;
	int t, normalized;

	for (t = 0; t < ARRAY_SIZE(types); t++) {
		for (normalized = 0; normalized < (types[t] == GL_FLOAT ? 1 : 2); normalized++) {
			pass = test_array(types[t], normalized)
				&& pass;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	Prog = piglit_build_simple_program(vertShaderText, fragShaderText);
	if (!Prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(Prog);

	ExpectedUniform = glGetUniformLocation(Prog, "expected");
	ToleranceUniform = glGetUniformLocation(Prog, "tolerance");
	AttrAttrib = glGetAttribLocation(Prog, "attr");
}
