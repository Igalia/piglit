/*
 * Copyright © 2012 Intel Corporation
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

/** @file maxblocks.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "There is a set of implementation-dependent maximums for the
 *      number of active uniform blocks used by each shader (vertex,
 *      fragment, geometry).  If the number of uniform blocks used by
 *      any shader in the program exceeds its corresponding limit, the
 *      program will fail to link.  The limits for vertex, fragment,
 *      and geometry shaders can be obtained by calling GetIntegerv
 *      with <pname> values of MAX_VERTEX_UNIFORM_BLOCKS,
 *      MAX_FRAGMENT_UNIFORM_BLOCKS, and MAX_GEOMETRY_UNIFORM_BLOCKS,
 *      respectively.
 *
 *      Additionally, there is an implementation-dependent limit on
 *      the sum of the number of active uniform blocks used by each
 *      shader of a program.  If a uniform block is used by multiple
 *      shaders, each such use counts separately against this combined
 *      limit.  The combined uniform block use limit can be obtained
 *      by calling GetIntegerv with a <pname> of
 *      MAX_COMBINED_UNIFORM_BLOCKS."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static char *
get_shader(GLenum target, const char *block_prefix, int blocks)
{
	char *shader = NULL;
	const char *vs_source =
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"\n"
		"varying vec4 v;"
		"\n"
		"%s"
		"\n"
		"void main() {\n"
		"	gl_Position = gl_Vertex;\n"
		"	v = vec4(0)%s;\n"
		"}\n";
	const char *fs_source =
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"\n"
		"varying vec4 v;"
		"\n"
		"%s"
		"\n"
		"void main() {\n"
		"	gl_FragColor = v%s;\n"
		"}\n";
	const char *prefix_template =
		"layout(std140) uniform %s_block%d {\n"
		"	vec4 %s_var%d;\n"
		"};\n";
	const char *body_template =
		" + %s_var%d";
	char *prefix, *prefix_tail, *body, *body_tail;
	int i;

	prefix = calloc(1, (strlen(block_prefix) * 2 + strlen(prefix_template) +
			    20) * blocks + 1);
	body = calloc(1, (strlen(block_prefix) + strlen(body_template) +
			  20) * blocks + 1);
	prefix_tail = prefix;
	prefix[0] = 0;
	body_tail = body;
	body[0] = 0;

	for (i = 0; i < blocks; i++) {
		prefix_tail += sprintf(prefix_tail, prefix_template,
				       block_prefix, i,
				       block_prefix, i);
		body_tail += sprintf(body_tail, body_template,
				     block_prefix, i);
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		asprintf(&shader, vs_source, prefix, body);
		break;
	case GL_FRAGMENT_SHADER:
		asprintf(&shader, fs_source, prefix, body);
		break;
	default:
		piglit_report_result(PIGLIT_FAIL);
	}

	free(prefix);
	free(body);

	return shader;
}

static GLuint
build_shaders(const char *vs_prefix, int vs_blocks,
	      const char *fs_prefix, int fs_blocks)
{
	char *vs_source, *fs_source;
	GLuint vs, fs, prog;

	vs_source = get_shader(GL_VERTEX_SHADER, vs_prefix, vs_blocks);
	fs_source = get_shader(GL_FRAGMENT_SHADER, fs_prefix, fs_blocks);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	if (!piglit_link_check_status_quiet(prog)) {
		glDeleteProgram(prog);
		prog = 0;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return prog;
}

static bool
fail_link_test(const char *vs_prefix, int vs_blocks,
	       const char *fs_prefix, int fs_blocks)
{
	GLuint prog;

	prog = build_shaders(vs_prefix, vs_blocks,
			     fs_prefix, fs_blocks);

	if (prog) {
		printf("linked with (%d, %d) blocks, should have failed\n",
		       vs_blocks, fs_blocks);
		return false;
	}

	glDeleteProgram(prog);
	return true;
}

static bool
test_draw(int y_index, GLuint prog,
	  GLuint *bos, int test_block, int active_blocks)
{
	float black[4] = {0, 0, 0, 0};
	/* Color values have to be 0 or 1, since in the case of a
	 * shared block between VS and FS, they'll be added twice.
	 */
	float other_colors[][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{1, 1, 1, 1},
	};
	float *color;
	int screen_w = 10;
	int screen_h = 10;
	int screen_x = screen_w * (1 + 2 * test_block);
	int screen_y = screen_h * (1 + 2 * y_index);
	float x = -1.0 + 2.0 * screen_x / piglit_width;
	float y = -1.0 + 2.0 * screen_y / piglit_height;
	float w = 2.0 * screen_w / piglit_width;
	float h = 2.0 * screen_h / piglit_height;
	int i;
	float *expected_color;

	expected_color = other_colors[test_block % ARRAY_SIZE(other_colors)];

	for (i = 0; i < active_blocks; i++) {
		if (i == test_block)
			color = expected_color;
		else
			color = black;

		glBindBuffer(GL_UNIFORM_BUFFER, bos[i]);
		glBufferData(GL_UNIFORM_BUFFER,
			     4 * sizeof(float), color, GL_DYNAMIC_DRAW);
	}

	piglit_draw_rect(x, y, w, h);

	if (screen_x + screen_w >= piglit_width ||
	    screen_y + screen_h >= piglit_height) {
		return true;
	}

	return piglit_probe_rect_rgba(screen_x, screen_y,
				      screen_w, screen_h, expected_color);
}

static bool
pass_link_test(int y_index,
	       const char *vs_prefix, int vs_blocks,
	       const char *fs_prefix, int fs_blocks)
{
	bool pass = true;
	GLuint prog;
	GLuint *bos = (GLuint *) calloc(vs_blocks + fs_blocks, sizeof(GLuint));
	GLint active_blocks;
	int i;

	prog = build_shaders(vs_prefix, vs_blocks,
			     fs_prefix, fs_blocks);

	if (!prog) {
		printf("shader with (%d, %d) blocks failed to link\n",
		       vs_blocks, fs_blocks);
		free(bos);
		return false;
	}

	glUseProgram(prog);
	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCKS, &active_blocks);
	glGenBuffers(active_blocks, bos);
	for (i = 0; i < active_blocks; i++) {
		glUniformBlockBinding(prog, i, i);
		glBindBufferBase(GL_UNIFORM_BUFFER, i, bos[i]);
	}

	for (i = 0; i < active_blocks; i++) {
		if (!test_draw(y_index, prog, bos, i, active_blocks)) {
			pass = false;
		}
	}

	glDeleteBuffers(active_blocks, bos);
	glDeleteProgram(prog);

	free(bos);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLint max_vs, max_fs, max_combined;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &max_vs);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &max_fs);
	glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &max_combined);
	printf("Max VS uniform blocks: %d\n", max_vs);
	printf("Max FS uniform blocks: %d\n", max_fs);
	printf("Max combined uniform blocks: %d\n", max_combined);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	pass = fail_link_test("vs", max_vs + 1,
			      "vs", 0) && pass;
	pass = fail_link_test("fs", 0,
			      "fs", max_fs + 1) && pass;
	if (max_vs + max_fs > max_combined) {
		pass = fail_link_test("vs",
				      max_vs,
				      "fs",
				      max_combined + 1 - max_vs) && pass;

		pass = fail_link_test("shared",
				      max_vs,
				      "shared",
				      max_combined + 1 - max_vs) && pass;
	}

	pass = pass_link_test(0,
			      "vs", max_vs,
			      "vs", 0) && pass;

	pass = pass_link_test(1,
			      "fs", 0,
			      "fs", max_fs) && pass;

	pass = pass_link_test(2,
			      "vs", max_vs,
			      "fs", MIN2(max_fs,
					 max_combined - max_vs)) && pass;

	pass = pass_link_test(3,
			      "shared", max_vs,
			      "shared", MIN2(max_fs,
					     max_combined - max_vs)) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
