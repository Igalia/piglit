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
 * Test glVertexAttribPointer with all combinations of types, sizes and
 * normalized/unnormalized.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
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
	"      color = vec4(1, 0, 0, 0); // bad! \n"
	"   else \n"
	"      color = vec4(0, 1, 0, 0); // good! \n"
	"} \n";

static const char *fragShaderText =
	"varying vec4 color;\n"
	"void main()\n"
	"{ \n"
	"   gl_FragColor = color; \n"
	"} \n";


static const GLfloat float4_data[] = { -0.5, 0.0, 0.75, 1.0 };
static const GLdouble double4_data[] = { -0.5, 0.0, 0.75, 1.0 };
static const GLubyte ubyte4_data[] = { 100, 0, 200, 255 };
static const GLbyte byte4_data[] = { 50, 0, -25, -50 };
static const GLushort ushort4_data[] = { 16000, 0, 32000, 65535 };
static const GLshort short4_data[] = { 2000, 0, -4000, -8000 };
static const GLuint uint4_data[] = { 10000000, 0, 20000000, 80000000 };
static const GLint int4_data[] = { 10000000, 0, -20000000, -40000000 };

static GLuint Prog;
static GLint ToleranceUniform, ExpectedUniform, AttrAttrib;


/*
 * Test glVertexAttribArray(type, size, normalized)
 */
static bool
test_array(GLenum type, GLuint size, GLboolean normalized)
{
	static const GLfloat verts[4][2] = {
		{ -1.0, -1.0 },
		{ 1.0, -1.0 },
		{ 1.0, 1.0 },
		{ -1.0, 1.0 }
	};
	static const GLfloat green[4] = { 0.0, 1.0, 0.0, 0.0 };
	GLubyte attr_buffer[4 * 4 * sizeof(double)];
	float maxVal;
	int typeSize;
	const void *data;
	float expected[4];
	int i, p;
	float tolerance;

	switch (type) {
	case GL_BYTE:
		maxVal = 127.0;
		typeSize = sizeof(GLbyte);
		data = byte4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) byte4_data[i];
		break;
	case GL_UNSIGNED_BYTE:
		maxVal = 255.0;
		typeSize = sizeof(GLubyte);
		data = ubyte4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) ubyte4_data[i];
		break;
	case GL_SHORT:
		maxVal = 32767.0;
		typeSize = sizeof(GLshort);
		data = short4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) short4_data[i];
		break;
	case GL_UNSIGNED_SHORT:
		maxVal = 65535.0;
		typeSize = sizeof(GLushort);
		data = ushort4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) ushort4_data[i];
		break;
	case GL_INT:
		maxVal = (float) 0x7fffffff;
		typeSize = sizeof(GLint);
		data = int4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) int4_data[i];
		break;
	case GL_UNSIGNED_INT:
		maxVal = (float) 0xffffffff;
		typeSize = sizeof(GLuint);
		data = uint4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) uint4_data[i];
		break;
	case GL_FLOAT:
		maxVal = 1.0;
		typeSize = sizeof(GLfloat);
		data = float4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) float4_data[i];
		break;
	case GL_DOUBLE:
		maxVal = 1.0;
		typeSize = sizeof(GLdouble);
		data = double4_data;
		for (i = 0; i < 4; i++)
			expected[i] = (float) double4_data[i];
		break;
	default:
		assert(0);
		maxVal = 1.0;
		typeSize = sizeof(GLfloat);
		data = NULL;
	}

	if (normalized) {
		for (i = 0; i < 4; i++) {
			expected[i] /= maxVal;
		}
	}

	/* set unused components to defaults */
	switch (size) {
	case 1:
		expected[1] = 0.0;
	case 2:
		expected[2] = 0.0;
	case 3:
		expected[3] = 1.0;
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

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(AttrAttrib, size, type,
			      normalized, 0, attr_buffer);
	glEnableVertexAttribArray(AttrAttrib);

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

	p = piglit_probe_pixel_rgba(piglit_width / 2, piglit_height / 2,
				    green);
	if (!p) {
		printf("Test %s[%d] %s failed\n",
		       piglit_get_gl_enum_name(type),
		       size,
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
                GL_DOUBLE
	};
	bool pass = true;
	int t, size, normalized;

	for (t = 0; t < ARRAY_SIZE(types); t++) {
		for (size = 1; size <= 4; size++) {
			for (normalized = 0; normalized < 2; normalized++) {
				pass = test_array(types[t], size, normalized)
					&& pass;
			}
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
