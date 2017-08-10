/**
 * Copyright 2017 Intel Corporation
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

/** @file compare-framebuffer-parameter-with-get.
 *
 * OpenGL 4.5 spec introduced new valid pnames for
 * GetFramebufferParameter. From OpenGL 4.5 spec, Section 9.2.3
 * "Framebuffer Object Queries":
 *
 *   "pname may also be one of DOUBLEBUFFER, IMPLEMENTATION_COLOR_-
 *    READ_FORMAT, IMPLEMENTATION_COLOR_READ_TYPE, SAMPLES,
 *    SAMPLE_BUFFERS, or STEREO, indicating the corresponding
 *    framebuffer-dependent state from table 23.73. Values of
 *    framebuffer-dependent state are identical to those that would be
 *    obtained were the framebuffer object bound and queried using the
 *    simple state queries in that table. These values may be queried
 *    from either a framebuffer object or a default framebuffer."
 *
 * That "simple state queries in that table" are either glGetBooleanv
 * or glGetIntegerv.
 *
 * 4.5 also defines a new method, available on previous versions
 * through the direct state access extension,
 * GetNamedFramebufferParameteriv:
 *
 * "For GetFramebufferParameteriv, the framebuffer object is that
 *  bound to target"
 *
 * "For GetNamedFramebufferParameteriv, framebuffer may be zero,
 *  indicating the default draw framebuffer, or the name of the
 *  framebuffer object."
 *
 * So with the Named version, you can query the same info, but you can
 * query for a framebuffer not bound at that moment.
 *
 * This test checks that the behaviour of GetFramebufferParameter,
 * GetNamedFramebufferParameter and glGetX is the same for the bound
 * framebuffer (default or user defined). Behaviour in the sense of
 * same value returned or same error generated. For *Named* we will
 * explicitly bound to a different framebuffer, to ensure that it
 * works when the queried framebuffer is not bound at that moment.
 *
 * Note that we will not check if the error or the value is correct,
 * just that are the same. Value and error correctness should be
 * evaluated by other tests.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 45;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/*
 * Values of table 23.73, defined on 4.5 spec, allowed on
 * GetFramebufferParameteriv (so the full table minus SAMPLE_POSITION)
 */
static const GLenum table_23_73_allowed[] = {
	GL_IMPLEMENTATION_COLOR_READ_FORMAT,
	GL_IMPLEMENTATION_COLOR_READ_TYPE,
	GL_DOUBLEBUFFER,
	GL_STEREO,
	GL_SAMPLE_BUFFERS,
	GL_SAMPLES,
};

GLuint framebuffers[3];
bool filter_pname = false;
GLenum global_pname;
bool filter_framebuffer = false;
int global_framebuffer;

/*
 * Returns if any of the table_23_73 enums is a boolean or not
 */
static bool
is_boolean(GLenum pname)
{
	switch(pname) {
	case GL_DOUBLEBUFFER:
	case GL_STEREO:
		return true;
	default:
		return false;
	}
}

static void
print_usage()
{
	printf("Usage: gl-4.5-compare-framebuffer-parameter-with-get <pname> <framebuffer>\n");
	printf("\tpname: only test this pname from table 23.73 (minus "
	       "SAMPLE_POSITION) to use. Optional. \n");
	printf("\tframebuffer: only test this framebuffer. Optional. Allowed values:\n "
	       "\t\t 0 (default framebuffer)\n"
	       "\t\t 1 (incomplete framebuffer)\n"
	       "\t\t 2 (complete framebuffer)\n");
}

static void
parse_args(int argc, char **argv)
{
	int i;
	bool found = false;

	if (argc > 3) {
		printf("Only two possible params supported\n");
		goto bad_params;
	}

	if (argc == 1)
		return;

	filter_pname = true;
	/* Note that this call will abort if the enum is not recognized */
	global_pname = piglit_get_gl_enum_from_name(argv[1]);
	for (i = 0; i < ARRAY_SIZE(table_23_73_allowed); i++) {
		if (global_pname == table_23_73_allowed[i]) {
			found = true;
			break;
		}
	}

	if (!found) {
		printf("pname %s is not valid for this test\n", argv[1]);
		goto bad_params;
	}

	if (argc == 2)
		return;

	filter_framebuffer = true;
	global_framebuffer = atoi(argv[2]);
	if (global_framebuffer < 0 || global_framebuffer > 2) {
		printf("Wrong value for framebuffer: %i\n", global_framebuffer);
		goto bad_params;
	}

	return;
bad_params:
	print_usage();
	piglit_report_result(PIGLIT_FAIL);
}

/*
 * This method calls wraps glGetBooleanv and glGetInteger, as
 * depending of the pname you will call one or the other. It also does
 * the boolean to integer casting, as GetFramebufferParameteriv
 * returns always int.
 */
static void
call_get_x(GLenum pname,
	   GLint *value,
	   GLenum *error)
{
	if (is_boolean(pname)) {
		GLboolean local_value;
		glGetBooleanv(pname, &local_value);
		*value = local_value;
	} else {
		GLint local_value;
		glGetIntegerv(pname, &local_value);
		*value = local_value;
	}

	*error = glGetError();
}

static const char*
get_framebuffer_name(int index)
{
	switch(index) {
	case 0:
		return "default framebuffer";
	case 1:
		return "incomplete framebuffer";
	case 2:
		return "complete framebuffer";
	default:
		assert(!"unknown framebuffer");
		return "unknown framebuffer";
	}
}

/*
 * Gets a framebuffer and attachs to it renderbuffer and other stuff,
 * in order to ensure that it is a complete framebuffer.
 *
 * returns if it was successful.
 */
static bool
complete_framebuffer(GLint fb)
{
	GLuint rb;

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_R8, 1, 2);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);

	if (GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER) &&
	    glGetError() == GL_NO_ERROR) {
		return true;
	} else {
		return false;
	}
}

/*
 * We pass the index inside the array of available framebuffers,
 * instead of the fb itself, because we also want to test binding with
 * a different valid fb.
 */
static bool
execute_subtest(int index,
		GLenum pname)
{
	GLint get_value;
	GLint parameter_value;
	GLint named_value;
	GLenum get_error;
	GLenum parameter_error;
	GLenum named_error;
	bool subtest_pass;
	GLuint fb;
	GLuint other_fb;

	fb = framebuffers[index];
	other_fb = framebuffers[(index + 1) % ARRAY_SIZE(framebuffers)];

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGetFramebufferParameteriv(GL_FRAMEBUFFER, pname, &parameter_value);
	parameter_error = glGetError();

	/*
	 * We re-bind to a different (but valid) framebuffer, as we
	 * want to check that NamedFramebufferParameter gets the same
	 * value even if other framebuffer is bound.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, other_fb);
	glGetNamedFramebufferParameteriv(fb, pname, &named_value);
	named_error = glGetError();

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	call_get_x(pname, &get_value, &get_error);

	subtest_pass = (get_error == parameter_error &&
			get_value == parameter_value &&
			parameter_error == named_error &&
			parameter_value == named_value);

	if (!subtest_pass) {
		printf("Different behaviour for pname %s.\n\tGetBooleanv/Integerv"
		       " returns %i and generate the error %s.\n"
		       "\tGetFramebufferParameter returns %i and generate the "
		       "error %s.\n\tGetNamedFramebufferParameter returns %i "
		       "and generate the error %s\n",
		       piglit_get_gl_enum_name(pname),
		       get_value, piglit_get_gl_error_name(get_error),
		       parameter_value, piglit_get_gl_error_name(parameter_error),
		       named_value, piglit_get_gl_error_name(named_error));
	}

	return subtest_pass;
}


enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	/*
	 * We don't check for framebuffer object extension support an
	 * any other, as we are already asking core version 4.5 on
	 * PIGLIT CONFIG
	 */
	parse_args(argc, argv);

	bool pass = true;
	int i;
	int c;
	bool subtest_pass;

	framebuffers[0] = 0;
	glCreateFramebuffers(2, &framebuffers[1]);
	piglit_check_gl_error(GL_NO_ERROR);

	if (!complete_framebuffer(framebuffers[2])) {
		printf("Not able to allocate a complete framebuffer\n");

		piglit_report_result(PIGLIT_FAIL);
	}

	for (c = 0; c < ARRAY_SIZE(framebuffers); c++) {

		if (filter_framebuffer && global_framebuffer != c)
			continue;

		for (i = 0; i < ARRAY_SIZE(table_23_73_allowed); i++) {
                        GLenum pname = table_23_73_allowed[i];

			if (filter_pname && global_pname != pname)
				continue;

			subtest_pass = execute_subtest(c, pname);

			PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "%s pname %s",
						 get_framebuffer_name(c),
						 piglit_get_gl_enum_name(pname));
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
