/*
 * Copyright © 2015 Intel Corporation
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

/** @file max-ssbo-size.c
 *
 * Tests linking and drawing with shader storage buffer objects of size
 * MAX_SHADER_STORAGE_BLOCK_SIZE.
 *
 * Based on ARB_uniform_buffer_object's maxuniformblocksize.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static enum {
	VS,
	VS_EXCEED,
	FS,
	FS_EXCEED,
} mode;

static void
usage(const char *name)
{
	fprintf(stderr, "usage: %s <vs | vsexceed | fs | fsexceed>\n",
		name);
	piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	const char *vs_ssbo_template =
		"#extension GL_ARB_shader_storage_buffer_object : enable\n"
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"\n"
		"varying vec4 vary;"
		"\n"
		"layout(std140) buffer ssbo {\n"
		"	vec4 v[%d];\n"
		"};\n"
		"uniform int i;\n"
		"\n"
		"void main() {\n"
		"	gl_Position = gl_Vertex;\n"
		"	vary = v[i];\n"
		"}\n";

	const char *fs_template =
		"#extension GL_ARB_shader_storage_buffer_object : enable\n"
		"\n"
		"varying vec4 vary;"
		"\n"
		"void main() {\n"
		"	gl_FragColor = vary;\n"
		"}\n";

	const char *vs_template =
		"#extension GL_ARB_shader_storage_buffer_object : enable\n"
		"\n"
		"void main() {\n"
		"	gl_Position = gl_Vertex;\n"
		"}\n";

	const char *fs_ssbo_template =
		"#extension GL_ARB_shader_storage_buffer_object : enable\n"
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"\n"
		"layout(std140) buffer ssbo {\n"
		"	vec4 v[%d];\n"
		"};\n"
		"uniform int i;\n"
		"\n"
		"void main() {\n"
		"	gl_FragColor = v[i];\n"
		"}\n";

	char *vs_source, *fs_source;
	GLint max_size, vec4s, i_location;
	GLuint vs, fs, prog, bo;
	GLenum target;
	float *data;
	size_t size;
	bool pass = true;
	bool link_should_fail;
	const float green[4] = { 0, 1, 0, 0 };
	int test_index;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_size);
	printf("Max shader storage block size: %d\n", max_size);
	vec4s = max_size / 4 / 4;

	switch (mode) {
	case VS:
		target = GL_VERTEX_SHADER;
		link_should_fail = false;
		test_index = vec4s - 1;
		break;
	case VS_EXCEED:
		target = GL_VERTEX_SHADER;
		link_should_fail = true;
		vec4s++;
		test_index = vec4s - 2;
		break;
	case FS:
		target = GL_FRAGMENT_SHADER;
		link_should_fail = false;
		test_index = vec4s - 1;
		break;
	case FS_EXCEED:
		target = GL_FRAGMENT_SHADER;
		link_should_fail = true;
		vec4s++;
		test_index = vec4s - 2;
		break;
	default:
		assert(false);
		target = GL_NONE;
		link_should_fail = false;
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		asprintf(&vs_source, vs_ssbo_template, vec4s);
		asprintf(&fs_source, "%s", fs_template);
		printf("Testing VS with shader storage block vec4 v[%d]\n", vec4s);
		break;
	case GL_FRAGMENT_SHADER:
		asprintf(&vs_source, "%s", vs_template);
		asprintf(&fs_source, fs_ssbo_template, vec4s);
		printf("Testing FS with shader storage block vec4 v[%d]\n", vec4s);
		break;
	default:
		piglit_report_result(PIGLIT_FAIL);
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	if (link_should_fail) {
		if (!piglit_link_check_status_quiet(prog)) {
			printf("Failed to link with shader storage block vec4 "
			       "v[%d]\n", vec4s);
			piglit_report_result(PIGLIT_PASS);
		}
	} else {
		if (!piglit_link_check_status_quiet(prog)) {
			fprintf(stderr,
				"Failed to link with shader storage block vec4 "
				"v[%d]\n", vec4s);
			return PIGLIT_FAIL;
		}
	}

	size = vec4s * 4 * sizeof(float);
	glGenBuffers(1, &bo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	memset(data, 0, size);

	/* The whole shader storage buffer will be zeros, except for the
	 * entry at v[test_index] which will be green.
	 */
	data[test_index * 4 + 0] = green[0];
	data[test_index * 4 + 1] = green[1];
	data[test_index * 4 + 2] = green[2];
	data[test_index * 4 + 3] = green[3];
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glUseProgram(prog);
	i_location = glGetUniformLocation(prog, "i");
	glUniform1i(i_location, test_index);

	glShaderStorageBlockBinding(prog, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bo);
	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	glDeleteProgram(prog);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if (argc < 2)
		usage(argv[0]);

	if (strcmp(argv[1], "vs") == 0)
		mode = VS;
	else if (strcmp(argv[1], "vsexceed") == 0)
		mode = VS_EXCEED;
	else if (strcmp(argv[1], "fs") == 0)
		mode = FS;
	else if (strcmp(argv[1], "fsexceed") == 0)
		mode = FS_EXCEED;
	else
		usage(argv[0]);
}
