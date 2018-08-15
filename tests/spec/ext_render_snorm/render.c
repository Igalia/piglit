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
 * Basic tests for formats added by GL_EXT_render_snorm extension
 *
 * https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_render_snorm.txt
 *
 * Test includes:
 * 	- texture uploads
 * 	- mipmap generation
 * 	- framebuffer creation
 * 	- rendering to
 * 	- reading from
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

#define PIGLIT_RESULT(x) x ? PIGLIT_PASS : PIGLIT_FAIL

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
} tests[] = {
	{ GL_R8_SNORM,		GL_RED,		1, },
	{ GL_RG8_SNORM,		GL_RG,		2, },
	{ GL_RGBA8_SNORM,	GL_RGBA,	4, },
};

static GLuint prog;

static void
upload(const struct fmt_test *test, void *data)
{
	glTexStorage2D(GL_TEXTURE_2D, 4, test->iformat, piglit_width,
		       piglit_height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, piglit_width,
			piglit_height, test->base_format, GL_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

static void
value_for_format(const struct fmt_test *test, void *value)
{
	unsigned short val = SCHAR_MAX;

	char *v = value;
	/* red */
	v[0] = val;
	/* green */
	if (test->bpp > 1) {
		v[0] = 0;
		v[1] = val;
	}
	/* blue */
	if (test->bpp > 2) {
		v[0] = 0;
		v[1] = 0;
		v[2] = val;
		v[3] = val;
	}
}

static void
generate_data(const struct fmt_test *test)
{
	unsigned pixels = piglit_width * piglit_height;
	void *data = malloc(pixels * test->bpp);

	char *p = data;
	for (unsigned i = 0; i < pixels; i++, p += test->bpp)
		value_for_format(test, p);

	upload(test, data);
	free(data);
}

static GLuint
create_and_bind_empty_texture()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return tex;
}

static GLuint
create_and_bind_texture(const struct fmt_test *test)
{
	GLuint tex = create_and_bind_empty_texture();
	generate_data(test);
	return tex;
}

static GLuint
create_and_bind_rbo(const struct fmt_test *test)
{
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, test->iformat, piglit_width,
			      piglit_height);
	return rbo;
}

static GLuint
create_and_bind_fbo(const struct fmt_test *test, GLuint *tex)
{
	GLuint fbo;
	GLuint fbo_tex = create_and_bind_empty_texture(test);
	upload(test, NULL);

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
	void *pix = malloc(amount * 4);
	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, GL_BYTE, pix);

	char value[4] = { 0, 0, 0, SCHAR_MAX };
	value_for_format(test, value);

	char *p = pix;
	for (unsigned i = 0; i < amount; i++, p += 4) {
		if (memcmp(p, value, sizeof(value)) == 0)
			continue;

                fprintf(stderr, "value:\n%d % d %d %d\nexpect:\n%d %d %d %d",
                        p[0], p[1], p[2], p[3],
                        value[0], value[1], value[2], value[3]);

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
test_format(const struct fmt_test *test)
{
	bool pass = true;

	glUseProgram(prog);
	glUniform1i(0 /* explicit loc */, 0);

	/* Test glRenderbufferStorage. */
	GLuint rbo = create_and_bind_rbo(test);
	if (!rbo || !piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "format 0x%x RBO test",
					     test->iformat);
		pass &= false;
	} else {
		piglit_report_subtest_result(PIGLIT_PASS,
					     "format 0x%x RBO test",
					     test->iformat);
	}
	glDeleteRenderbuffers(1, &rbo);

	/* Create framebuffer object. */
	GLuint fbo_tex;
	const GLuint fbo = create_and_bind_fbo(test, &fbo_tex);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "format 0x%x fbo fail",
					     test->iformat);
		pass &= false;
	}

	/* Create a texture, upload data */
	const GLuint texture = create_and_bind_texture(test);

	render_texture(texture, GL_TEXTURE_2D, fbo);

	glDeleteTextures(1, &texture);

	/* Test glCopyTexImage2D by copying current fbo content to
	 * a texture, rendering copy back to fbo and verifying fbo contents.
	 */
	GLuint tmp_tex = create_and_bind_empty_texture();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, test->iformat, 0, 0, piglit_width,
			 piglit_height, 0);

	render_texture(tmp_tex, GL_TEXTURE_2D, fbo);

	glDeleteTextures(1, &tmp_tex);

	/* Verify contents. */
	pass &= verify_contents(test);

	glDeleteFramebuffers(1, &fbo);

	/* Render fbo contents to window. */
	render_texture(fbo_tex, GL_TEXTURE_2D, 0);

	piglit_present_results();

	glDeleteTextures(1, &fbo_tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      vertex_data);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (void*) (vertex_data + (2 * sizeof(float))));

	bool pass = true;

	const struct fmt_test *test = tests;

	/* Loop over each format. */
	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++, test++) {
		bool fmt_pass = test_format(test);
		piglit_report_subtest_result(PIGLIT_RESULT(fmt_pass),
					     "format 0x%x",
					     test->iformat);
		pass &= fmt_pass;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_RESULT(pass);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_render_snorm");
	prog = piglit_build_simple_program(vs_source, fs_source);
}
