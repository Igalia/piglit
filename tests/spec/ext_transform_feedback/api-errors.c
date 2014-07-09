/*
 * Copyright © 2011 Intel Corporation
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
 * \file api-errors.c
 *
 * Test that the implementation flags various
 * transform-feedback-related error conditions.
 *
 * This test covers all of the error conditions as specified in the
 * "Errors" section of the EXT_transform_feedback spec, with the
 * following exceptions:
 *
 * - Errors related to BeginQuery and EndQuery.
 *
 * - Error due to mismatch of transform feedback mode and drawing mode
 *   (e.g. drawing GL_LINES when transform feedback is expecting
 *   GL_TRIANGLES).
 *
 * - Errors due to exceeding MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTIBS in
 *   a call to TransformFeedbackVaryings, GetTransformFeedbackVarying,
 *   or
 *   Get{Integer,Boolean}Indexedv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING).
 *
 * In addition, there are a few tests which verify that errors do not
 * occur during normal operation ("interleaved_ok_*", "separate_ok_*",
 * and "link_other_active").  These tests help to verify that the
 * implementation is not overly aggressive in flagging errors.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define XFB_BUFFER_SIZE 12
#define NUM_BUFFERS 2

enum test_mode {
	NORMAL,
	NO_VARYINGS,
	UNBOUND_BUFFER,
	SKIP_USE_PROGRAM,
	BEGIN_ACTIVE,
	USEPROG_ACTIVE,
	LINK_CURRENT_ACTIVE,
	LINK_OTHER_ACTIVE,
	BIND_ACTIVE,
	END_INACTIVE,
	BIND_MAX,
	BIND_BAD_SIZE,
	BIND_BAD_OFFSET,
	NOT_A_PROGRAM,
	USEPROGSTAGE_ACTIVE,
	USEPROGSTAGE_NOACTIVE,
	BIND_PIPELINE
};

enum bind_mode {
	BASE,
	RANGE,
	OFFSET,
};

static const char *vstext =
	"varying vec4 foo;\n"
	"varying vec4 bar;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  foo = vec4(1.0);\n"
	"  bar = vec4(1.0);\n"
	"  gl_Position = vec4(1.0);\n"
	"}\n";

static const char *vstext_sep_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects : enable\n"
	"#if __VERSION__ > 140\n"
	"/* At least some versions of AMD's closed-source driver\n"
	" * contain a bug that requires redeclaration of gl_PerVertex\n"
	" * interface block in core profile shaders.\n"
	" */\n"
	"out gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"};\n"
	"#endif\n"
	"varying vec4 foo;\n"
	"varying vec4 bar;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  foo = vec4(1.0);\n"
	"  bar = vec4(1.0);\n"
	"  gl_Position = vec4(1.0);\n"
	"}\n";

static const char *varyings[] = { "foo", "bar" };

static struct test_desc
{
	const char *name;
	enum test_mode mode;
	int param;
	enum bind_mode bind_mode;
	GLenum buffer_mode;
	int num_buffers;
	GLboolean skip_use_program;
} tests[] = {
	/* name                      mode                 param                               num_buffers
	 *                                                    bind_mode
	 *                                                            buffer_mode
	 */
	{ "interleaved_ok_base",     NORMAL,               0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "interleaved_ok_range",    NORMAL,               0, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "interleaved_ok_offset",   NORMAL,               0, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "interleaved_unbound",     UNBOUND_BUFFER,       0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "interleaved_no_varyings", NO_VARYINGS,          0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "separate_ok_1",           NORMAL,               0, BASE,   GL_SEPARATE_ATTRIBS,    1 },
	{ "separate_unbound_0_1",    UNBOUND_BUFFER,       0, BASE,   GL_SEPARATE_ATTRIBS,    1 },
	{ "separate_ok_2",           NORMAL,               0, BASE,   GL_SEPARATE_ATTRIBS,    2 },
	{ "separate_unbound_0_2",    UNBOUND_BUFFER,       0, BASE,   GL_SEPARATE_ATTRIBS,    2 },
	{ "separate_unbound_1_2",    UNBOUND_BUFFER,       1, BASE,   GL_SEPARATE_ATTRIBS,    2 },
	{ "separate_no_varyings",    NO_VARYINGS,          0, BASE,   GL_SEPARATE_ATTRIBS,    1 },
	{ "no_prog_active",          SKIP_USE_PROGRAM,     0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "begin_active",            BEGIN_ACTIVE,         0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "useprog_active",          USEPROG_ACTIVE,       0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "link_current_active",     LINK_CURRENT_ACTIVE,  0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "link_other_active",       LINK_OTHER_ACTIVE,    0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_base_active",        BIND_ACTIVE,          0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_active",       BIND_ACTIVE,          0, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_active",      BIND_ACTIVE,          0, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "end_inactive",            END_INACTIVE,         0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_base_max",           BIND_MAX,             0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_max",          BIND_MAX,             0, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_max",         BIND_MAX,             0, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_m4",      BIND_BAD_SIZE,       -4, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_0",       BIND_BAD_SIZE,        0, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_1",       BIND_BAD_SIZE,        1, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_2",       BIND_BAD_SIZE,        2, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_3",       BIND_BAD_SIZE,        3, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_size_5",       BIND_BAD_SIZE,        5, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_offset_1",     BIND_BAD_OFFSET,      1, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_offset_2",     BIND_BAD_OFFSET,      2, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_offset_3",     BIND_BAD_OFFSET,      3, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_range_offset_5",     BIND_BAD_OFFSET,      5, RANGE,  GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_offset_1",    BIND_BAD_OFFSET,      1, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_offset_2",    BIND_BAD_OFFSET,      2, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_offset_3",    BIND_BAD_OFFSET,      3, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_offset_offset_5",    BIND_BAD_OFFSET,      5, OFFSET, GL_INTERLEAVED_ATTRIBS, 1 },
	{ "not_a_program",           NOT_A_PROGRAM,        0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "useprogstage_noactive",   USEPROGSTAGE_NOACTIVE,0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "useprogstage_active",     USEPROGSTAGE_ACTIVE,  0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },
	{ "bind_pipeline",           BIND_PIPELINE,        0, BASE,   GL_INTERLEAVED_ATTRIBS, 1 },

};

static void
do_bind(const struct test_desc *test, GLuint buf, int i)
{
	int size = test->mode == BIND_BAD_SIZE
		? test->param : sizeof(float[XFB_BUFFER_SIZE]);
	int offset = test->mode == BIND_BAD_OFFSET
		? test->param : 0;

	switch (test->bind_mode) {
	case BASE:
		printf("BindBufferBase(buffer %i)\n", i);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, buf);
		break;
	case RANGE:
		printf("BindBufferRange(buffer %i, offset=%i, size=%i)\n", i,
		       offset, size);
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, i, buf,
				  offset, size);
		break;
	case OFFSET:
		printf("BindBufferOffsetEXT(buffer %i, offset=%i)\n", i,
		       offset);
		glBindBufferOffsetEXT(GL_TRANSFORM_FEEDBACK_BUFFER, i, buf,
				      offset);
		break;
	}
}

static GLboolean
do_test(const struct test_desc *test)
{
	GLuint vs;
	GLuint progs[2];
	GLuint pipes[2];
	GLuint bufs[NUM_BUFFERS];
	float initial_xfb_buffer_contents[XFB_BUFFER_SIZE];
	GLboolean pass = GL_TRUE;
	int i;
	int num_varyings = test->mode == NO_VARYINGS ? 0 : test->num_buffers;
	GLint max_separate_attribs;
	char* vstext_sep;

	if (test->mode == USEPROGSTAGE_ACTIVE
	    || test->mode == USEPROGSTAGE_NOACTIVE
	    || test->mode == BIND_PIPELINE) {
		piglit_require_extension("GL_ARB_separate_shader_objects");

		if (piglit_get_gl_version() >= 32)
			asprintf(&vstext_sep, vstext_sep_template, 150);
		else
			asprintf(&vstext_sep, vstext_sep_template, 110);
	}

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
		      &max_separate_attribs);
	printf("MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTIBS=%i\n",
	       max_separate_attribs);

	printf("Compile vertex shader\n");
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	if (test->mode == USEPROGSTAGE_ACTIVE
	    || test->mode == USEPROGSTAGE_NOACTIVE
	    || test->mode == BIND_PIPELINE) {
		/* Note, we can't use glCreateShaderProgramv because the setup
		 * of transform feedback must be done before linking
		 */
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext_sep);
		progs[0] = glCreateProgram();
		glProgramParameteri(progs[0], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(progs[0], vs);
	} else if (test->mode == NOT_A_PROGRAM) {
		printf("Create a program and then delete it\n");
		progs[0] = glCreateProgram();
		glDeleteProgram(progs[0]);
	} else {
		progs[0] = glCreateProgram();
		glAttachShader(progs[0], vs);
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	printf("Setup transform feedback for %i varyings in %s mode\n",
	       num_varyings,
	       test->buffer_mode == GL_INTERLEAVED_ATTRIBS
	       ? "interleaved" : "separate");
	glTransformFeedbackVaryings(progs[0], num_varyings,
				    varyings, test->buffer_mode);

	if (test->mode == NOT_A_PROGRAM) {
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		return pass;
	}

	printf("Link program\n");
	glLinkProgram(progs[0]);
	pass = piglit_link_check_status(progs[0]) && pass;

	if (test->mode == USEPROGSTAGE_ACTIVE
	    || test->mode == USEPROGSTAGE_NOACTIVE
	    || test->mode == BIND_PIPELINE) {
		printf("Create 2nd program for the pipeline\n");
		progs[1] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
						  (const char **) &vstext_sep);
		pass = piglit_link_check_status(progs[1]) && pass;
	}

	if (test->mode == USEPROG_ACTIVE || test->mode == LINK_OTHER_ACTIVE) {
		printf("Prepare 2nd program\n");
		progs[1] = glCreateProgram();
		glAttachShader(progs[1], vs);
	}
	if (test->mode == USEPROG_ACTIVE) {
		printf("Link 2nd program\n");
		glLinkProgram(progs[1]);
		pass = piglit_link_check_status(progs[1]) && pass;
	}

	if (test->mode == USEPROGSTAGE_ACTIVE
	    || test->mode == USEPROGSTAGE_NOACTIVE
	    || test->mode == BIND_PIPELINE) {
		printf("Use pipeline\n");
		glGenProgramPipelines(2, pipes);
		glUseProgramStages(pipes[0], GL_VERTEX_SHADER_BIT, progs[0]);
		glUseProgramStages(pipes[1], GL_VERTEX_SHADER_BIT, progs[1]);
		glBindProgramPipeline(pipes[0]);
	} else if (test->mode == SKIP_USE_PROGRAM) {
		printf("Don't use program\n");
	} else {
		printf("Use program\n");
		glUseProgram(progs[0]);
	}

	printf("Prepare %i buffers\n", test->num_buffers);
	glGenBuffers(test->num_buffers, bufs);
	memset(initial_xfb_buffer_contents, 0,
	       sizeof(initial_xfb_buffer_contents));
	for (i = 0; i < test->num_buffers; ++i) {
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, bufs[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     sizeof(initial_xfb_buffer_contents),
			     initial_xfb_buffer_contents, GL_STREAM_READ);
	}

	switch (test->mode) {
	case BIND_MAX:
		do_bind(test, bufs[0], max_separate_attribs);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		return pass;
	case BIND_BAD_SIZE:
	case BIND_BAD_OFFSET:
		do_bind(test, bufs[0], 0);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		return pass;
	default:
		break;
	}

	for (i = 0; i < test->num_buffers; ++i) {
		if (test->mode == UNBOUND_BUFFER && i == test->param) {
			printf("Don't bind buffer %i\n", i);
		} else {
			do_bind(test, bufs[i], i);
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (test->mode == END_INACTIVE) {
		printf("EndTransformFeedback\n");
		glEndTransformFeedback();
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		return pass;
	}

	printf("BeginTransformFeedback\n");
	glBeginTransformFeedback(GL_POINTS);
	switch (test->mode) {
	case UNBOUND_BUFFER:
	case NO_VARYINGS:
	case SKIP_USE_PROGRAM:
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	default:
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		break;
	}

	switch (test->mode) {
	case BEGIN_ACTIVE:
		printf("BeginTransformFeedback\n");
		glBeginTransformFeedback(GL_POINTS);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	case USEPROG_ACTIVE:
		printf("Use new program\n");
		glUseProgram(progs[1]);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	case LINK_CURRENT_ACTIVE:
		printf("Link current program\n");
		glLinkProgram(progs[0]);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	case LINK_OTHER_ACTIVE:
		printf("Link 2nd program\n");
		glLinkProgram(progs[1]);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		break;
	case BIND_ACTIVE:
		do_bind(test, bufs[0], 0);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	case USEPROGSTAGE_ACTIVE:
		printf("Use new program stage\n");
		glUseProgramStages(pipes[0], GL_VERTEX_SHADER_BIT, progs[1]);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	case USEPROGSTAGE_NOACTIVE:
		printf("Use new program stage\n");
		glUseProgramStages(pipes[1], GL_VERTEX_SHADER_BIT, progs[1]);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		break;
	case BIND_PIPELINE:
		printf("Bind a new pipeline\n");
		glBindProgramPipeline(pipes[1]);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		break;
	default:
		break;
	}

	return pass;
}

void
print_usage_and_exit(const char *prog_name)
{
	int i;

	printf("Usage: %s <test_name>\n"
	       "  where <test_name> is one of:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); ++i)
		printf("    %s\n", tests[i].name);
	exit(0);
}

const struct test_desc *
find_matching_test(const char *prog_name, const char *test_name)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		if (strcmp(tests[i].name, test_name) == 0)
			return &tests[i];
	}
	print_usage_and_exit(prog_name);
	return NULL; /* won't actually be reached */
}

void
piglit_init(int argc, char **argv)
{
	const struct test_desc *test;

	/* Parse params. */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	test = find_matching_test(argv[0], argv[1]);

	piglit_require_GLSL();
	piglit_require_transform_feedback();
	if (test->bind_mode == OFFSET) {
		/* BindBufferOffset only exists in the EXT specification */
		piglit_require_extension("GL_EXT_transform_feedback");
	}

	piglit_report_result(do_test(test) ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
