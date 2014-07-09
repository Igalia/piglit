/* Copyright © 2012 Intel Corporation
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

/** @file minmax.c
 *
 * Test for the minimum maximum value in the GL_ARB_texture_buffer_object spec.
 */

#include "piglit-util-gl.h"

enum channels {
	A,
	L,
	LA,
	I,
	R,
	RG,
	RGB,
	RGBA,
};

static const struct format {
	GLenum format;
	GLenum type;
	int components;
	bool norm;
	enum channels channels;
} formats[] = {
	{ GL_ALPHA8,                     GL_UNSIGNED_BYTE,  1,        true,     A },
	{ GL_ALPHA16,                    GL_UNSIGNED_SHORT, 1,        true,     A },
	{ GL_ALPHA16F_ARB,               GL_HALF_FLOAT,     1,        false,    A },
	{ GL_ALPHA32F_ARB,               GL_FLOAT,          1,        false,    A },
	{ GL_ALPHA8I_EXT,                GL_BYTE,           1,        false,    A },
	{ GL_ALPHA16I_EXT,               GL_SHORT,          1,        false,    A },
	{ GL_ALPHA32I_EXT,               GL_INT,            1,        false,    A },
	{ GL_ALPHA8UI_EXT,               GL_UNSIGNED_BYTE,  1,        false,    A },
	{ GL_ALPHA16UI_EXT,              GL_UNSIGNED_SHORT, 1,        false,    A },
	{ GL_ALPHA32UI_EXT,              GL_UNSIGNED_INT,   1,        false,    A },

	{ GL_LUMINANCE8,                 GL_UNSIGNED_BYTE,  1,        true,     L },
	{ GL_LUMINANCE16,                GL_UNSIGNED_SHORT, 1,        true,     L },
	{ GL_LUMINANCE16F_ARB,           GL_HALF_FLOAT,     1,        false,    L },
	{ GL_LUMINANCE32F_ARB,           GL_FLOAT,          1,        false,    L },
	{ GL_LUMINANCE8I_EXT,            GL_BYTE ,          1,        false,    L },
	{ GL_LUMINANCE16I_EXT,           GL_SHORT,          1,        false,    L },
	{ GL_LUMINANCE32I_EXT,           GL_INT  ,          1,        false,    L },
	{ GL_LUMINANCE8UI_EXT,           GL_UNSIGNED_BYTE,  1,        false,    L },
	{ GL_LUMINANCE16UI_EXT,          GL_UNSIGNED_SHORT, 1,        false,    L },
	{ GL_LUMINANCE32UI_EXT,          GL_UNSIGNED_INT,   1,        false,    L },

	{ GL_LUMINANCE8_ALPHA8,          GL_UNSIGNED_BYTE,  2,        true,     LA },
	{ GL_LUMINANCE16_ALPHA16,        GL_UNSIGNED_SHORT, 2,        true,     LA },
	{ GL_LUMINANCE_ALPHA16F_ARB,     GL_HALF_FLOAT,     2,        false,    LA },
	{ GL_LUMINANCE_ALPHA32F_ARB,     GL_FLOAT,          2,        false,    LA },
	{ GL_LUMINANCE_ALPHA8I_EXT,      GL_BYTE,           2,        false,    LA },
	{ GL_LUMINANCE_ALPHA16I_EXT,     GL_SHORT,          2,        false,    LA },
	{ GL_LUMINANCE_ALPHA32I_EXT,     GL_INT,            2,        false,    LA },
	{ GL_LUMINANCE_ALPHA8UI_EXT,     GL_UNSIGNED_BYTE,  2,        false,    LA },
	{ GL_LUMINANCE_ALPHA16UI_EXT,    GL_UNSIGNED_SHORT, 2,        false,    LA },
	{ GL_LUMINANCE_ALPHA32UI_EXT,    GL_UNSIGNED_INT,   2,        false,    LA },

	{ GL_INTENSITY8,                 GL_UNSIGNED_BYTE,  1,        true,     I },
	{ GL_INTENSITY16,                GL_UNSIGNED_SHORT, 1,        true,     I },
	{ GL_INTENSITY16F_ARB,           GL_HALF_FLOAT,     1,        false,    I },
	{ GL_INTENSITY32F_ARB,           GL_FLOAT,          1,        false,    I },
	{ GL_INTENSITY8I_EXT,            GL_BYTE,           1,        false,    I },
	{ GL_INTENSITY16I_EXT,           GL_SHORT,          1,        false,    I },
	{ GL_INTENSITY32I_EXT,           GL_INT,            1,        false,    I },
	{ GL_INTENSITY8UI_EXT,           GL_UNSIGNED_BYTE,  1,        false,    I },
	{ GL_INTENSITY16UI_EXT,          GL_UNSIGNED_SHORT, 1,        false,    I },
	{ GL_INTENSITY32UI_EXT,          GL_UNSIGNED_INT,   1,        false,    I },

	{ GL_RGBA8,                      GL_UNSIGNED_BYTE,  4,        true,     RGBA },
	{ GL_RGBA16,                     GL_UNSIGNED_SHORT, 4,        true,     RGBA },
	{ GL_RGBA16F,                    GL_HALF_FLOAT,     4,        false,    RGBA },
	{ GL_RGBA32F,                    GL_FLOAT,          4,        false,    RGBA },
	{ GL_RGBA8I_EXT,                 GL_BYTE,           4,        false,    RGBA },
	{ GL_RGBA16I_EXT,                GL_SHORT,          4,        false,    RGBA },
	{ GL_RGBA32I_EXT,                GL_INT,            4,        false,    RGBA },
	{ GL_RGBA8UI_EXT,                GL_UNSIGNED_BYTE,  4,        false,    RGBA },
	{ GL_RGBA16UI_EXT,               GL_UNSIGNED_SHORT, 4,        false,    RGBA },
	{ GL_RGBA32UI_EXT,               GL_UNSIGNED_INT,   4,        false,    RGBA },

	/* These don't appear in the GL_ARB_texture_buffer_object or
	 * GL_ARB_texture_rg specs, but they do appear in the GL 3.1
	 * specification's table for buffer texture formats.  We
	 * assume that the intent was for RG to be included even in
	 * ARB_tbo + ARB_texture_rg
	 */
	{ GL_R8,                         GL_UNSIGNED_BYTE,  1,        true,     R },
	{ GL_R16,                        GL_UNSIGNED_SHORT, 1,        true,     R },
	{ GL_R16F,                       GL_HALF_FLOAT,     1,        false,    R },
	{ GL_R32F,                       GL_FLOAT,          1,        false,    R },
	{ GL_R8I,                        GL_BYTE,           1,        false,    R },
	{ GL_R16I,                       GL_SHORT,          1,        false,    R },
	{ GL_R32I,                       GL_INT,            1,        false,    R },
	{ GL_R8UI,                       GL_UNSIGNED_BYTE,  1,        false,    R },
	{ GL_R16UI,                      GL_UNSIGNED_SHORT, 1,        false,    R },
	{ GL_R32UI,                      GL_UNSIGNED_INT,   1,        false,    R },

	{ GL_RG8,                        GL_UNSIGNED_BYTE,  2,        true,     RG },
	{ GL_RG16,                       GL_UNSIGNED_SHORT, 2,        true,     RG },
	{ GL_RG16F,                      GL_HALF_FLOAT,     2,        false,    RG },
	{ GL_RG32F,                      GL_FLOAT,          2,        false,    RG },
	{ GL_RG8I,                       GL_BYTE,           2,        false,    RG },
	{ GL_RG16I,                      GL_SHORT,          2,        false,    RG },
	{ GL_RG32I,                      GL_INT,            2,        false,    RG },
	{ GL_RG8UI,                      GL_UNSIGNED_BYTE,  2,        false,    RG },
	{ GL_RG16UI,                     GL_UNSIGNED_SHORT, 2,        false,    RG },
	{ GL_RG32UI,                     GL_UNSIGNED_INT,   2,        false,    RG },

	{ GL_RGB32F,                     GL_FLOAT,          3,        false,    RGB },
	{ GL_RGB32I,                     GL_INT,            3,        false,    RGB },
	{ GL_RGB32UI,                    GL_UNSIGNED_INT,   3,        false,    RGB },
};

bool test_vs;
bool test_arb;
bool test_rgb32;
struct program {
	GLuint prog;
	int pos_location;
	int expected_location;
};

struct program prog_f;
struct program prog_i;
struct program prog_u;
static int vertex_location;

static int y_index;

static uint8_t uint8_data[] = {
	0x00, 0x01, 0x02, 0x03,
	0x10, 0x20, 0x30, 0x40,
	0x60, 0x90, 0xa0, 0xff,
};

static uint16_t uint16_data[] = {
	0x0000, 0x0001, 0x0002, 0x0003,
	0x4000, 0x8000, 0xc000, 0xffff,
};

static uint32_t uint32_data[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003,
	0x40000000, 0x80000000, 0xc0000000, 0xffffffff,
};

static float float_data[] = {
	 0.0,  0.25, 0.5,  0.75,
	 1.0,  2.0,  3.0,  4.0,
	-1.0, -2.0, -3.0, -4.0,
};

static float
transform_x(float x)
{
	return -1.0 + 2.0 * x / piglit_width;
}

static float
transform_y(float y)
{
	return -1.0 + 2.0 * y / piglit_height;
}

static bool
get_expected_f(const struct format *format, int sample, float *expected)
{
	float chans[4] = { 0 };
	int i;

	for (i = 0; i < format->components; i++) {
		int comp = sample * format->components + i;

		switch (format->type) {
		case GL_FLOAT:
			chans[i] = float_data[comp];
			break;
		case GL_HALF_FLOAT:
			chans[i] = float_data[comp];
			break;
		case GL_UNSIGNED_BYTE:
			chans[i] = uint8_data[comp] / 255.0;
			break;
		case GL_UNSIGNED_SHORT:
			chans[i] = uint16_data[comp] / 65535.0;
			break;
		default:
			printf("line %d, bad type: %s\n", __LINE__,
			       piglit_get_gl_enum_name(format->type));
			memset(expected, 0, 16);
			return false;
		}
	}

	switch (format->channels) {
	case RGBA:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = chans[2];
		expected[3] = chans[3];
		break;
	case RGB:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = chans[2];
		expected[3] = 1.0;
		break;
	case RG:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = 0.0;
		expected[3] = 1.0;
		break;

	case R:
		expected[0] = chans[0];
		expected[1] = 0.0;
		expected[2] = 0.0;
		expected[3] = 1.0;
		break;

	case A:
		expected[0] = 0.0;
		expected[1] = 0.0;
		expected[2] = 0.0;
		expected[3] = chans[0];
		break;

	case L:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = 1.0;
		break;

	case LA:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = chans[1];
		break;

	case I:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = chans[0];
		break;
	}

	return true;
}

static bool
get_expected_i(const struct format *format, int sample, uint32_t *expected)
{
	uint32_t chans[4];
	int i;

	for (i = 0; i < format->components; i++) {
		int comp = sample * format->components + i;

		switch (format->type) {
		case GL_BYTE:
			chans[i] = ((int8_t *)uint8_data)[comp];
			break;
		case GL_UNSIGNED_BYTE:
			chans[i] = uint8_data[comp];
			break;
		case GL_SHORT:
			chans[i] = ((int16_t *)uint16_data)[comp];
			break;
		case GL_UNSIGNED_SHORT:
			chans[i] = uint16_data[comp];
			break;
		case GL_INT:
		case GL_UNSIGNED_INT:
			chans[i] = uint32_data[comp];
			break;
		default:
			printf("line %d, bad type: %s\n", __LINE__,
			       piglit_get_gl_enum_name(format->type));
			memset(expected, 0, 16);
			return false;
		}
	}

	switch (format->channels) {
	case RGBA:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = chans[2];
		expected[3] = chans[3];
		break;
	case RGB:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = chans[2];
		expected[3] = 1.0;
		break;
	case RG:
		expected[0] = chans[0];
		expected[1] = chans[1];
		expected[2] = 0.0;
		expected[3] = 1.0;
		break;

	case R:
		expected[0] = chans[0];
		expected[1] = 0.0;
		expected[2] = 0.0;
		expected[3] = 1.0;
		break;

	case A:
		expected[0] = 0.0;
		expected[1] = 0.0;
		expected[2] = 0.0;
		expected[3] = chans[0];
		break;

	case L:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = 1.0;
		break;

	case LA:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = chans[1];
		break;

	case I:
		expected[0] = chans[0];
		expected[1] = chans[0];
		expected[2] = chans[0];
		expected[3] = chans[0];
		break;
	}

	return true;
}

enum piglit_result
test_format(int format_index)
{
	const struct format *format = &formats[format_index];
	GLuint tex, bo;
	bool is_rg = (format->channels == R ||
		      format->channels == RG);
	bool is_arb = (format->channels == I ||
		       format->channels == L ||
		       format->channels == LA ||
		       format->channels == A);
	bool is_rgb32 = (format->channels == RGB);
	bool pass = true;
	int data_components, num_samples;
	int i;
	bool returns_float = (format->norm ||
			      format->type == GL_FLOAT ||
			      format->type == GL_HALF_FLOAT);
	bool returns_int = (!format->norm &&
			    (format->type == GL_BYTE ||
			     format->type == GL_SHORT ||
			     format->type == GL_INT));
	bool returns_uint = (!format->norm &&
			     (format->type == GL_UNSIGNED_BYTE ||
			      format->type == GL_UNSIGNED_SHORT ||
			      format->type == GL_UNSIGNED_INT));
	struct program *prog;

	if (returns_float)
		prog = &prog_f;
	else if (returns_int)
		prog = &prog_i;
	else
		prog = &prog_u;

	glUseProgram(prog->prog);

	if (test_arb != is_arb)
		return PIGLIT_SKIP;

	if (is_rgb32 && !test_rgb32)
		return PIGLIT_SKIP;

	/* These didn't exist in the extension before being promoted to
	 * GL 3.1.
	 */
	if (is_rg && piglit_get_gl_version() < 31)
		return PIGLIT_SKIP;

	printf("Testing %s\n", piglit_get_gl_enum_name(format->format));

	glGenBuffers(1, &bo);
	glBindBuffer(GL_TEXTURE_BUFFER, bo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glTexBuffer(GL_TEXTURE_BUFFER, format->format, bo);

	switch (format->type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		glBufferData(GL_TEXTURE_BUFFER, sizeof(uint8_data), uint8_data,
			     GL_STATIC_READ);
		data_components = ARRAY_SIZE(uint8_data);
		break;

	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_data),
			     uint16_data, GL_STATIC_READ);
		data_components = ARRAY_SIZE(uint16_data);
		break;

	case GL_INT:
	case GL_UNSIGNED_INT:
		glBufferData(GL_TEXTURE_BUFFER, sizeof(uint32_data),
			     uint32_data, GL_STATIC_READ);
		data_components = ARRAY_SIZE(uint32_data);
		break;

	case GL_FLOAT:
		glBufferData(GL_TEXTURE_BUFFER, sizeof(float_data), float_data,
			     GL_STATIC_READ);
		data_components = ARRAY_SIZE(float_data);
		break;

	case GL_HALF_FLOAT: {
		unsigned short hf_data[ARRAY_SIZE(float_data)];
		for (i = 0; i < ARRAY_SIZE(float_data); i++) {
			hf_data[i] = piglit_half_from_float(float_data[i]);
		}
		glBufferData(GL_TEXTURE_BUFFER, sizeof(hf_data), hf_data,
			     GL_STATIC_READ);
		data_components = ARRAY_SIZE(float_data);

		break;
	}

	default:
		printf("line %d, bad type: %s\n", __LINE__,
		       piglit_get_gl_enum_name(format->type));
		return PIGLIT_SKIP;
	}

	num_samples = data_components / format->components;

	for (i = 0; i < num_samples; i++) {
		float x1 = 5 + i * 10;
		float x2 = 10 + i * 10;
		float y1 = piglit_height - (5 + y_index * 10);
		float y2 = piglit_height - (10 + y_index * 10);
		GLfloat verts[8] = {
			transform_x(x1), transform_y(y1),
			transform_x(x2), transform_y(y1),
			transform_x(x2), transform_y(y2),
			transform_x(x1), transform_y(y2),
		};
		float expected_f[4];
		uint32_t expected_i[4];
		const float green[4] = {0, 1, 0, 0};

		if (returns_float) {
			if (!get_expected_f(format, i, expected_f))
				return PIGLIT_SKIP;
			glUniform4fv(prog->expected_location, 1, expected_f);
		} else {
			if (!get_expected_i(format, i, expected_i))
				return PIGLIT_SKIP;
			if (returns_uint) {
				glUniform4uiv(prog->expected_location, 1,
					      expected_i);
			} else {
				glUniform4iv(prog->expected_location, 1,
					     (int *)expected_i);
			}
		}

		glUniform1i(prog->pos_location, i);

		glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(verts), verts,
			     GL_STREAM_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		if (pass &&
		    !piglit_probe_rect_rgba(x1, y2,
					    x2 - x1, y1 - y2, green)) {
			if (returns_int) {
				printf("     Texel: %d %d %d %d\n",
				       expected_i[0], expected_i[1],
				       expected_i[2], expected_i[3]);
			} else if (returns_uint) {
				printf("     Texel: %u %u %u %u\n",
				       expected_i[0], expected_i[1],
				       expected_i[2], expected_i[3]);
			} else {
				printf("     Texel: %f %f %f %f\n",
				       expected_f[0], expected_f[1],
				       expected_f[2], expected_f[3]);
			}
			pass = false;
		}
	}

	glDeleteBuffers(1, &bo);
	glDeleteTextures(1, &tex);

	glUseProgram(0);
	y_index++;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s", piglit_get_gl_enum_name(format->format));
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_SKIP;
	int i;
	GLuint vao, vbo;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	y_index = 0;

	/* For GL core, we need to have a vertex array object bound.
	 * Otherwise, we don't particularly have to.  Always use a
	 * vertex buffer object, though.
	 */
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo);
	if (piglit_get_gl_version() >= 31) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertex_location);

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		piglit_merge_result(&result, test_format(i));
	}

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	piglit_present_results();

	return result;
}

static char *vs_vert_source =
	"#version 140\n"
	"in vec4 vertex;\n"
	"out vec4 color;\n"
	"uniform %ssamplerBuffer s;\n"
	"uniform int pos;\n"
	"uniform %svec4 expected;"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"\n"
	"	%svec4 result = texelFetch(s, pos);\n"
	"	%svec4 delta = result - expected;\n"
	"	bvec4 fail = greaterThanEqual(abs(delta), %s);\n"
	"	if (any(fail)) {\n"
	"		color = 0.25 + 0.5 * vec4(fail);\n"
	"	} else {\n"
	"		color = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"	}\n"
	"}\n";

static char *fs_vert_source =
	"#version 140\n"
	"in vec4 color;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";


static char *vs_frag_source =
	"#version 140\n"
	"in vec4 vertex;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"}\n";

static char *fs_frag_source =
	"#version 140\n"
	"uniform %ssamplerBuffer s;\n"
	"uniform int pos;\n"
	"uniform %svec4 expected;"
	"void main()\n"
	"{\n"
	"	%svec4 result = texelFetch(s, pos);\n"
	"	%svec4 delta = result - expected;\n"
	"	bvec4 fail = greaterThanEqual(abs(delta), %s);\n"
	"	if (any(fail)) {\n"
	"		gl_FragColor = 0.25 + 0.5 * vec4(fail);\n"
	"	} else {\n"
	"		gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"	}\n"
	"}\n";

static void
create_program(struct program *program, const char *type)
{
	char *fs_source, *vs_source;
	GLuint prog;
	char *threshold;

	if (strcmp(type, "") == 0)
		threshold = "vec4(0.02)";
	else
		threshold = "ivec4(1)";

	if (test_vs) {
		asprintf(&vs_source, vs_vert_source, type, type, type, type,
			 threshold);
		fs_source = fs_vert_source;
	} else {
		vs_source = vs_frag_source;
		asprintf(&fs_source, fs_frag_source, type, type, type, type,
			 threshold);
	}

	prog = piglit_build_simple_program(vs_source, fs_source);

	program->prog = prog;
        program->pos_location = glGetUniformLocation(prog, "pos");
        program->expected_location = glGetUniformLocation(prog,
							       "expected");
	vertex_location = glGetAttribLocation(prog, "vertex");
	assert(vertex_location == 0);
}

static void
init_programs()
{
	create_program(&prog_f, "");
	create_program(&prog_i, "i");
	create_program(&prog_u, "u");
}

static void
usage(const char *name)
{
	printf("usage: %s <fs | vs> <core | arb>\n", name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(140);

	piglit_require_extension("GL_EXT_texture_integer");
	piglit_require_extension("GL_ARB_texture_rg");

	if (test_arb) {
		piglit_require_extension("GL_ARB_texture_buffer_object");
	} else {
		if (piglit_get_gl_version() < 31)
			piglit_require_extension("GL_ARB_texture_buffer_object");

		if (piglit_is_extension_supported("GL_ARB_texture_buffer_object_rgb32"))
			test_rgb32 = true;

	}

	init_programs();
}

PIGLIT_GL_TEST_CONFIG_BEGIN
	test_vs = PIGLIT_STRIP_ARG("vs");
	if (!test_vs && !PIGLIT_STRIP_ARG("fs"))
		usage(argv[0]);

	test_arb = PIGLIT_STRIP_ARG("arb");
	if (!test_arb && !PIGLIT_STRIP_ARG("core"))
		usage(argv[0]);

	if (test_arb)
		config.supports_gl_compat_version = 10;
	else
		config.supports_gl_core_version = 31;

	config.window_width = 200;
	config.window_height = 500;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END
