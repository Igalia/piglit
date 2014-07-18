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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * Tests the formats of ARB_vertex_type_2_10_10_10 using
 * glVertexAttribPointer.  We test all combinations of GL_INT_2_10_10_10_REV
 * vs. GL_UNSIGNED_INT_2_10_10_10_REV and 4 vs. GL_BGRA and normalized vs.
 * unnormalized.
 *
 * Authors: Meng-Lin Wu, Brian Paul
 */


#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_width = 320;
	config.window_height = 60;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


struct test_info {
	GLenum size;
	GLenum type;
	GLboolean normalized;
	const char* name;
	float expected_color[4];
};

static const struct test_info tests[] = {
	{ 4,       GL_INT_2_10_10_10_REV,		GL_FALSE, "RGBA SINT",	{0.5, 0.25, 0, 1} },
	{ 4,       GL_INT_2_10_10_10_REV,		GL_TRUE,  "RGBA SNORM",	{0.5, 0.25, 0, 1} },
	{ 4,       GL_UNSIGNED_INT_2_10_10_10_REV,	GL_FALSE, "RGBA UINT",	{0.5, 0, 0.25, 1} },
	{ 4,       GL_UNSIGNED_INT_2_10_10_10_REV,	GL_TRUE,  "RGBA UNORM",	{0.5, 0, 0.25, 0.333} },
	{ GL_BGRA, GL_INT_2_10_10_10_REV,		GL_TRUE,  "BGRA SNORM",	{0, 0.25, 0.5, 1} },
	{ GL_BGRA, GL_UNSIGNED_INT_2_10_10_10_REV,	GL_TRUE,  "BGRA UNORM",	{0.25, 0, 0.5, 0.333} }
#if 0
	/* Not allowed, according to GL 3.3 core spec */
	{ GL_BGRA,	GL_INT_2_10_10_10_REV,		GL_FALSE, "BGRA SINT",	{0, 0.25, 0.5, 1} },
	{ GL_BGRA,	GL_UNSIGNED_INT_2_10_10_10_REV,	GL_FALSE, "BGRA UINT",	{0, 0.25, 0.5, 1} },
#endif
};


static const char *vertShaderText =
	"attribute vec4 vColor;\n"
	"varying vec4 vertColor;\n"
	"uniform float normFactor;\n"
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	vertColor.xyz = vColor.xyz / normFactor;\n"
	"	vertColor.w   = vColor.w;\n"
	"} \n";

static const char *fragShaderText =
	"varying vec4 vertColor;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vertColor;\n"
	"} \n";


static GLuint prog;
static GLint normFactor;


static int
i32to10(int x)
{
	if (x >= 0)
		return x & 0x1ff;
	else
		return 1024 - (abs(x) & 0x1ff);
}

static int
i32to2(int x)
{
	if (x >= 0)
		return x & 0x1;
	else
		return 1 - abs(x);
}


/**
 * Pack signed integer (x, y, z, w) into a 32-bit GLuint.
 */
static GLuint
iconv(GLint x, GLint y, GLint z, GLint w)
{
	unsigned val;

	val = i32to10(x);
	val |= i32to10(y) << 10;
	val |= i32to10(z) << 20;
	val |= i32to2(w) << 30;
	return val;
}


/**
 * Pack unsigned integer (x, y, z, w) into a 32-bit GLuint.
 */
static GLuint
uconv(GLuint x, GLuint y, GLuint z, GLuint w)
{
	return (((x & 0x3ff)) |
		((y & 0x3ff) << 10) |
		((z & 0x3ff) << 20) |
		((w & 0x3) << 30));
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_type_2_10_10_10_rev");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	prog = piglit_build_simple_program(vertShaderText, fragShaderText);
	glBindAttribLocation(prog, 1, "vColor");
	glLinkProgram(prog);
	glUseProgram(prog);

	normFactor = glGetUniformLocation(prog, "normFactor");
}


static bool
test(int x1, int y1, int x2, int y2, const struct test_info *test)
{
	GLuint v[3], c[3];
	bool pass;

	printf("testing: %s\n", test->name);

	/* vertex positions */
	v[0] = iconv(x1, y1, 0, 1);
	v[1] = iconv(x1, y2, 0, 1);
	v[2] = iconv(x2, y1, 0, 1);

	/* Setup c[] (color) array values */
	if (test->type == GL_INT_2_10_10_10_REV) {
		c[0] = c[1] = c[2] = iconv(511, 255, 0, 1);
		if (test->normalized == GL_TRUE)
			glUniform1f(normFactor, 2);
		else
			glUniform1f(normFactor, 1022);
	} else {
		assert(test->type == GL_UNSIGNED_INT_2_10_10_10_REV);
		c[0] = c[1] = c[2] = uconv(1023, 0, 511, 1);
		if (test->normalized == GL_TRUE)
			glUniform1f(normFactor, 2);
		else
			glUniform1f(normFactor, 2046);
	}

	/* Setup arrays and check for errors */
	glVertexAttribPointer(0, 4, GL_INT_2_10_10_10_REV, GL_FALSE, 0, v);
	if (!piglit_check_gl_error(GL_NO_ERROR))
	    return false;

	glVertexAttribPointer(1, test->size, test->type, test->normalized, 0, c);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("color array: size 0x%x, type %s, normalized %d, c %d, %d, %d\n",
		       test->size,
		       piglit_get_gl_enum_name(test->type),
		       test->normalized,
		       c[0], c[1], c[2]);
		return false;
	}

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	pass = piglit_probe_pixel_rgba(x1+5, y1+5, test->expected_color);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	unsigned i;
	int x = 0, y = 0;

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		pass = test(x, y, x+20, y+20, tests + i) && pass;
		x += 20;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
