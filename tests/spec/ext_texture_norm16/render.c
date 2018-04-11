/*
 * Copyright © 2018 Intel Corporation
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
 * @file
 * Basic tests for formats added by GL_EXT_texture_norm16 extension
 *
 * https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_norm16.txt
 *
 * Test includes:
 * 	- texture uploads
 * 	- mipmap generation
 * 	- framebuffer creation
 * 	- rendering to
 * 	- reading from
 *	- interaction with GL_EXT_copy_image
 *	- interaction with GL_OES_texture_buffer
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"#version 310 es\n"
	"layout(location = 0) in highp vec4 vertex;\n"
	"layout(location = 1) in highp vec4 uv;\n"
	"out highp vec2 tex_coord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"	tex_coord = uv.st;\n"
	"}\n";

static const char fs_source[] =
	"#version 310 es\n"
	"layout(location = 0) uniform sampler2D texture;\n"
	"in highp vec2 tex_coord;\n"
	"out highp vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = texture2D(texture, tex_coord);\n"
	"}\n";

static const char fs_buf_source[] =
	"#version 310 es\n"
	"#extension GL_OES_texture_buffer : require\n"
	"layout(location = 0) uniform highp samplerBuffer buf;\n"
	"in highp vec2 tex_coord;\n"
	"out highp vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = texelFetch(buf, 0);\n"
	"}\n";

/* trianglestrip, interleaved vertices + texcoords */
static const GLfloat vertex_data[] = {
	-1.0f,  1.0f,
	0.0f,  1.0f,
	1.0f,  1.0f,
	1.0f,  1.0f,
	-1.0f, -1.0f,
	0.0f,  0.0f,
	1.0f, -1.0f,
	1.0f,  0.0f
};

static const struct fmt_test {
	GLenum iformat;
	GLenum base_format;
	unsigned bpp;
	GLenum type;
	bool req_render;
	bool can_read;
} tests[] = {
	{ GL_R16_EXT,		GL_RED,		2,	GL_UNSIGNED_SHORT,	true,	true,	},
	{ GL_RG16_EXT,		GL_RG,		4,	GL_UNSIGNED_SHORT,	true,	true,	},
	{ GL_RGB16_EXT,		GL_RGB,		6,	GL_UNSIGNED_SHORT,	false,	true,	},
	{ GL_RGBA16_EXT,	GL_RGBA,	8,	GL_UNSIGNED_SHORT,	true,	true,	},
	{ GL_R16_SNORM_EXT,	GL_RED,		2,	GL_SHORT,		false,	false,	},
	{ GL_RG16_SNORM_EXT,	GL_RG,		4,	GL_SHORT,		false,	false,	},
	{ GL_RGB16_SNORM_EXT,	GL_RGB,		6,	GL_SHORT,		false,	false,	},
	{ GL_RGBA16_SNORM_EXT,	GL_RGBA,	8,	GL_SHORT,		false,	false,	},
};

static GLuint prog;
static GLuint buf_prog;

static void
upload(const struct fmt_test *test, void *data)
{
	/* glGenerateMipmap only for color renderable formats. */
	if (test->req_render) {
		glTexStorage2D(GL_TEXTURE_2D, 4, test->iformat, piglit_width,
			       piglit_height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, piglit_width,
				piglit_height, test->base_format, test->type,
				data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, test->iformat, piglit_width,
		     piglit_height, 0, test->base_format, test->type, data);
}

static unsigned
get_max_value(GLenum type)
{
	return type == GL_SHORT ? SHRT_MAX : USHRT_MAX;
}

static void
value_for_format(const struct fmt_test *test, unsigned short *value)
{
	unsigned short val = get_max_value(test->type);

	/* red */
	value[0] =  val;
	/* yellow */
	if (test->bpp > 2)
		value[1] = val;
	/* pink */
	if (test->bpp > 4) {
		value[2] = val;
		value[1] = 0;
	}
	/* blue */
	if (test->bpp > 6) {
		value[3] = val;
		value[0] = 0;
	}
}

static void
generate_data(const struct fmt_test *test)
{
	unsigned pixels = piglit_width * piglit_height;
	char *data = malloc (pixels * test->bpp);
	unsigned short *p = (unsigned short *) data;

	for (unsigned i = 0; i < pixels; i++, p += test->bpp / 2)
		value_for_format(test, p);

	upload(test, data);
	free(data);
}

static GLuint
create_texture(const struct fmt_test *test)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	generate_data(test);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return tex;
}

static GLuint
create_fbo(const struct fmt_test *test, GLuint *tex)
{
	GLuint fbo;
	GLuint fbo_tex = create_texture(test);

	*tex = fbo_tex;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, fbo_tex, 0);
	return fbo;
}

static void
render_texture(GLuint texture, GLenum target, GLuint fbo_target)
{
	glBindTexture(target, texture);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_target);

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static bool
verify_contents(const struct fmt_test *test)
{
	bool result = true;
	unsigned amount = piglit_width * piglit_height;
	unsigned short *pix = malloc (amount * 8);
	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA,
		     GL_UNSIGNED_SHORT, pix);

	/* Setup expected value, alpha is always max in the test. */
	unsigned short value[4] = { 0 };
	value_for_format(test, value);
	value[3] = get_max_value(test->type);

	unsigned short *p = pix;
	for (unsigned i = 0; i < amount; i++, p += 4) {
		if (memcmp(p, value, sizeof(value)) == 0)
			continue;

		piglit_report_subtest_result(PIGLIT_FAIL,
					     "format 0x%x read fail",
					     test->iformat);
		result = false;
		break;
	}

	free(pix);
	return result;
}

static bool
verify_contents_float(const struct fmt_test *test)
{
	/* Setup expected value, alpha is always max in the test. */
	unsigned short value[4] = { 0 };
	unsigned short max = get_max_value(test->type);
	value_for_format(test, value);
	value[3] = max;

	const float expected[4] = {
		value[0] / max,
		value[1] / max,
		value[2] / max,
		value[3] / max,
	};

	bool res = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
					  expected);

	if (!res)
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "format 0x%x read fail",
					     test->iformat);
	return res;
}

static bool
test_copy_image(const struct fmt_test *test, GLuint src, GLuint *texture)
{
	bool result = true;
	GLuint tex = create_texture(test);
	*texture = tex;
	glCopyImageSubData(src, GL_TEXTURE_2D, 0, 0, 0, 0, tex, GL_TEXTURE_2D,
			   0, 0, 0, 0, piglit_width, piglit_height, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "format 0x%x copyimage fail",
					     test->iformat);
		result = false;
	}
	return result;
}

static bool
buffer_test(const struct fmt_test *test)
{
	GLuint tex, tbo;

	/* Setup expected value, alpha is always max in the test. */
	unsigned short tbo_data[4] = { 0 };
	value_for_format(test, tbo_data);
	tbo_data[3] = get_max_value(test->type);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(tbo_data), tbo_data,
		     GL_STATIC_DRAW);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glTexBuffer(GL_TEXTURE_BUFFER, test->iformat, tbo);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glUseProgram(buf_prog);
	glUniform1i(0 /* explicit loc */, 0);

	render_texture(tex, GL_TEXTURE_BUFFER, 0);

	if (!verify_contents_float(test))
		return false;

	piglit_present_results();

	glDeleteTextures(1, &tex);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}

enum piglit_result
piglit_display(void)
{
	bool has_tbo =
		piglit_is_extension_supported("GL_OES_texture_buffer");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      vertex_data);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (void*) (vertex_data + (2 * sizeof(float))));

	bool pass = true;

	/* Loop over each format. */
	const struct fmt_test *test = tests;
	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++, test++) {

		/* The req_render formats match with formats that are
		 * supported by texture buffer objects.
		 */
		if (has_tbo && test->req_render) {
			bool buf_test = buffer_test(test);
			piglit_report_subtest_result(buf_test ? PIGLIT_PASS : PIGLIT_FAIL,
						     "format 0x%x TBO test",
						     test->iformat);
			pass &= buf_test;
		}

		glUseProgram(prog);
		glUniform1i(0 /* explicit loc */, 0);

		/* Create a texture, upload data */
		const GLuint texture = create_texture(test);

		glBindTexture(GL_TEXTURE_2D, texture);

		/* Can only texture from. */
		if (!test->req_render) {
			/* Render texture to window and verify contents. */
			render_texture(texture, GL_TEXTURE_2D, 0);
			pass &= verify_contents_float(test);
			piglit_present_results();
			if (pass)
				piglit_report_subtest_result(PIGLIT_PASS,
							     "format 0x%x",
							     test->iformat);
			glDeleteTextures(1, &texture);
			continue;
		}

		GLuint fbo_tex;
		const GLuint fbo = create_fbo(test, &fbo_tex);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
			GL_FRAMEBUFFER_COMPLETE) {
			piglit_report_subtest_result(PIGLIT_FAIL,
						     "format 0x%x fbo fail",
						     test->iformat);
			pass &= false;
		}

		render_texture(texture, GL_TEXTURE_2D, fbo);

		/* If GL_EXT_copy_image is supported then create another
		 * texture, copy contents and render result to fbo.
		 */
		GLuint texture_copy = 0;
		if (piglit_is_extension_supported("GL_EXT_copy_image")) {
			bool copy_pass =
				test_copy_image(test, texture, &texture_copy);
			pass &= copy_pass;
			piglit_report_subtest_result(copy_pass ?
						     PIGLIT_PASS : PIGLIT_FAIL,
						     "copy image format 0x%x",
						     test->iformat);
			render_texture(texture_copy, GL_TEXTURE_2D, fbo);
		}

		/* If format can be read, verify contents. */
		if (test->can_read)
			pass &= verify_contents(test);

		/* Render fbo contents to window. */
		render_texture(fbo_tex, GL_TEXTURE_2D, 0);

		piglit_present_results();

		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &texture);
		glDeleteTextures(1, &texture_copy);

		if (pass)
			piglit_report_subtest_result(PIGLIT_PASS,
						     "format 0x%x",
						     test->iformat);

	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_norm16");

	prog = piglit_build_simple_program(vs_source, fs_source);

	if (piglit_is_extension_supported("GL_OES_texture_buffer"))
		buf_prog = piglit_build_simple_program(vs_source, fs_buf_source);
}
